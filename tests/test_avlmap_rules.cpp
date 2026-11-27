// Tests for final_project_map::AvlMap (src/AvlMap.h), the team's hand-built AVL tree map.
// Covers the PDF's required edge cases (empty, one element, duplicates, removing the first,
// last and only element), AVL balance after every change, the comparator (std::less for asks,
// std::greater for bids), in-order visits for BOOK, counters, and a random comparison
// against std::map.
#include <functional>
#include <initializer_list>
#include <map>
#include <type_traits>

#include "doctest/doctest.h"
#include "AvlMap.h"

using final_project_map::AvlMap;

namespace {

// Counts how many copies of itself exist, to check every node is freed.
struct TrackedValue {
    static int live;
    TrackedValue() { ++live; }
    TrackedValue(const TrackedValue&) { ++live; }
    TrackedValue& operator=(const TrackedValue&) = default;
    ~TrackedValue() { --live; }
};
int TrackedValue::live = 0;

// Collects the keys in visiting order into out; returns how many.
template <class Map>
int keys_in_order(Map& m, int* out, int limit = 100000) {
    int n = 0;
    m.forEachInOrder([&](auto& entry) {
        if (n == limit) return false;
        out[n++] = entry.first;
        return true;
    });
    return n;
}

struct Lcg {
    unsigned long long state;
    explicit Lcg(unsigned long long seed) : state(seed) {}
    int next(int bound) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<int>((state >> 33) % static_cast<unsigned long long>(bound));
    }
};

}  // namespace

// ---------- empty map ----------

TEST_CASE("AvlMap: an empty map") {
    AvlMap<int, int> m;
    CHECK_FALSE(m.find(1));
    CHECK(m.at(1) == nullptr);
    CHECK(m.first() == nullptr);
    CHECK_FALSE(m.erase(1));
    int keys[1];
    CHECK(keys_in_order(m, keys) == 0);
    CHECK(m.check_invariants());
}

// ---------- one element ----------

TEST_CASE("AvlMap: one element") {
    AvlMap<int, int> m;
    m[5] = 50;
    CHECK(m.find(5));
    REQUIRE(m.at(5) != nullptr);
    CHECK(*m.at(5) == 50);
    REQUIRE(m.first() != nullptr);
    CHECK(m.first()->first == 5);
    CHECK(m.first()->second == 50);
    CHECK_FALSE(m.find(4));
    CHECK(m.check_invariants());
}

TEST_CASE("AvlMap: operator[] on a new key creates a default value") {
    AvlMap<int, int> m;
    CHECK(m[7] == 0);            // int() is 0
    CHECK(m.find(7));
}

// ---------- duplicates ----------

TEST_CASE("AvlMap: the same key twice is one entry, and keeps its value") {
    AvlMap<int, int> m;
    m[5] = 50;
    int& again = m[5];           // existing key: no new entry
    CHECK(again == 50);
    int keys[4];
    CHECK(keys_in_order(m, keys) == 1);
}

TEST_CASE("AvlMap: operator[] returns the same stored value each time") {
    AvlMap<int, int> m;
    int* first_address = &m[5];
    m[1] = 1;
    m[9] = 9;                    // other inserts and rotations
    CHECK(&m[5] == first_address);
}

TEST_CASE("AvlMap: values can be changed through at") {
    AvlMap<int, int> m;
    m[5] = 50;
    *m.at(5) = 55;
    CHECK(m[5] == 55);
}

// ---------- erase: first, last, only, root, missing ----------

TEST_CASE("AvlMap: erase") {
    AvlMap<int, int> m;
    for (int k = 1; k <= 7; ++k) m[k] = k * 10;          // 1..7

    SUBCASE("the first (smallest) key") {
        CHECK(m.erase(1));
        CHECK(m.first()->first == 2);
    }
    SUBCASE("the last (largest) key") {
        CHECK(m.erase(7));
        CHECK_FALSE(m.find(7));
    }
    SUBCASE("the root (a node with two children)") {
        CHECK(m.erase(4));                                // 1..7 inserted in order: 4 is the root
        CHECK_FALSE(m.find(4));
    }
    SUBCASE("a key that is not there") {
        CHECK_FALSE(m.erase(99));
    }
    SUBCASE("the same key twice") {
        CHECK(m.erase(3));
        CHECK_FALSE(m.erase(3));
    }
    SUBCASE("every key") {
        for (int k = 1; k <= 7; ++k) CHECK(m.erase(k));
        CHECK(m.first() == nullptr);
    }

    // runs after every subcase: still a valid AVL tree, still sorted, values still match keys
    CHECK(m.check_invariants());
    int keys[7];
    int n = keys_in_order(m, keys);
    for (int i = 0; i < n; ++i) {
        CHECK(*m.at(keys[i]) == keys[i] * 10);
        if (i > 0) CHECK(keys[i - 1] < keys[i]);
    }
}

TEST_CASE("AvlMap: erase the only element") {
    AvlMap<int, int> m;
    m[5] = 50;
    CHECK(m.erase(5));
    CHECK_FALSE(m.find(5));
    CHECK(m.first() == nullptr);
    CHECK(m.check_invariants());
}

TEST_CASE("AvlMap: erase relinks nodes, so other values keep their address") {
    AvlMap<int, int> m;
    for (int k = 1; k <= 100; ++k) m[k] = k;
    int* addresses[101];
    for (int k = 1; k <= 100; ++k) addresses[k] = m.at(k);
    for (int k = 1; k <= 100; k += 3) m.erase(k);         // many erases and rotations
    for (int k = 1; k <= 100; ++k) {
        if ((k - 1) % 3 == 0) continue;
        REQUIRE(m.at(k) == addresses[k]);
    }
    CHECK(m.check_invariants());
}

// ---------- balance ----------

TEST_CASE("AvlMap: stays balanced for increasing, decreasing and alternating keys") {
    AvlMap<int, int> m;
    SUBCASE("increasing (like W2 rising prices)") {
        for (int k = 0; k < 2000; ++k) { m[k] = k; REQUIRE(m.check_invariants()); }
    }
    SUBCASE("decreasing") {
        for (int k = 2000; k > 0; --k) { m[k] = k; REQUIRE(m.check_invariants()); }
    }
    SUBCASE("zig-zag (forces left-right and right-left rotations)") {
        for (int k = 0; k < 1000; ++k) {
            m[k] = k;
            m[-k - 1] = k;
            REQUIRE(m.check_invariants());
        }
    }
}

TEST_CASE("AvlMap: removing many keys keeps it balanced") {
    AvlMap<int, int> m;
    for (int k = 0; k < 1000; ++k) m[k] = k;
    for (int k = 0; k < 1000; k += 2) {
        REQUIRE(m.erase(k));
        REQUIRE(m.check_invariants());
    }
}

// ---------- comparator: asks (std::less) and bids (std::greater) ----------

TEST_CASE("AvlMap: with std::less, first() is the smallest key (best ask)") {
    AvlMap<double, int> asks;
    asks[50.30] = 1;
    asks[50.10] = 2;
    asks[50.20] = 3;
    REQUIRE(asks.first() != nullptr);
    CHECK(asks.first()->first == doctest::Approx(50.10));
}

TEST_CASE("AvlMap: with std::greater, first() is the largest key (best bid)") {
    AvlMap<double, int, std::greater<double>> bids;
    bids[50.00] = 1;
    bids[50.20] = 2;
    bids[50.10] = 3;
    REQUIRE(bids.first() != nullptr);
    CHECK(bids.first()->first == doctest::Approx(50.20));
    CHECK(bids.check_invariants());
}

TEST_CASE("AvlMap: with std::greater, in-order visits go largest first") {
    AvlMap<int, int, std::greater<int>> bids;
    for (int k : {3, 1, 4, 5, 2}) bids[k] = k;
    int keys[5];
    REQUIRE(keys_in_order(bids, keys) == 5);
    CHECK(keys[0] == 5);
    CHECK(keys[4] == 1);
}

// ---------- in-order visits (for BOOK) ----------

TEST_CASE("AvlMap: forEachInOrder visits keys in sorted order") {
    AvlMap<int, int> m;
    for (int k : {50, 30, 70, 20, 40, 60, 80}) m[k] = k;
    int keys[7];
    REQUIRE(keys_in_order(m, keys) == 7);
    int expected[7] = {20, 30, 40, 50, 60, 70, 80};
    for (int i = 0; i < 7; ++i) CHECK(keys[i] == expected[i]);
}

TEST_CASE("AvlMap: forEachInOrder stops when visit returns false (BOOK depth)") {
    AvlMap<int, int> m;
    for (int k = 1; k <= 100; ++k) m[k] = k;
    int keys[3];
    REQUIRE(keys_in_order(m, keys, 3) == 3);
    CHECK(keys[0] == 1);
    CHECK(keys[2] == 3);
}

// ---------- memory ----------

TEST_CASE("AvlMap: destructor and erase free every node") {
    TrackedValue::live = 0;
    {
        AvlMap<int, TrackedValue> m;
        for (int k = 0; k < 100; ++k) m[k];
        CHECK(TrackedValue::live == 100);
        m.erase(10);
        m.erase(50);
        CHECK(TrackedValue::live == 98);
    }
    CHECK(TrackedValue::live == 0);
}

TEST_CASE("AvlMap: cannot be copied") {
    static_assert(!std::is_copy_constructible<AvlMap<int, int>>::value,
                  "AvlMap must not be copyable (two maps would share nodes)");
    static_assert(!std::is_copy_assignable<AvlMap<int, int>>::value,
                  "AvlMap must not be copy-assignable (two maps would share nodes)");
    CHECK(true);
}

// ---------- counters ----------

TEST_CASE("AvlMap: counters record work and can be reset") {
    AvlMap<int, int> m;
    for (int k = 0; k < 10; ++k) m[k] = k;               // increasing keys force rotations
    m.find(5);
#if KIT_COUNTERS_ENABLED
    CHECK(m.stats().allocations == 10);
    CHECK(m.stats().comparisons > 0);
    CHECK(m.stats().visits > 0);
    CHECK(m.stats().extra.at("rotations") > 0);
#endif
    m.reset_stats();
    CHECK(m.stats().comparisons == 0);
    CHECK(m.stats().allocations == 0);
}

// ---------- compared against std::map (known correct) ----------

TEST_CASE("AvlMap: random inserts, erases and lookups agree with std::map") {
    AvlMap<int, int> m;
    std::map<int, int> expected;
    Lcg rng(42);
    for (int step = 0; step < 20000; ++step) {
        int key = rng.next(500);
        int op = rng.next(3);
        if (op == 0) {
            m[key] = step;
            expected[key] = step;
        } else if (op == 1) {
            REQUIRE(m.erase(key) == (expected.erase(key) == 1));
        } else {
            int* found = m.at(key);
            auto it = expected.find(key);
            REQUIRE((found != nullptr) == (it != expected.end()));
            if (found != nullptr) REQUIRE(*found == it->second);
        }
        if (step % 500 == 0) REQUIRE(m.check_invariants());
    }
    int keys[500];
    int n = keys_in_order(m, keys);
    REQUIRE(n == static_cast<int>(expected.size()));
    int i = 0;
    for (const auto& kv : expected) {
        CHECK(keys[i] == kv.first);
        CHECK(*m.at(kv.first) == kv.second);
        ++i;
    }
    if (!expected.empty()) CHECK(m.first()->first == expected.begin()->first);
}
