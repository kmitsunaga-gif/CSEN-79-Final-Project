// Tests for coen79_final::vector (src/vector.h), the team's hand-built dynamic array.
#include <stdexcept>
#include <string>
#include <utility>

#include "doctest/doctest.h"
#include "vector.h"

using coen79_final::vector;

// ---------- construction ----------

TEST_CASE("vector: a new vector is empty with the default capacity") {
    vector<int> v;
    CHECK(v.empty());
    CHECK(v.size() == 0);
    CHECK(v.capacity() == 2);
    CHECK(v.begin() == v.end());
}

TEST_CASE("vector: a starting capacity can be chosen, including 0") {
    vector<int> v(10);
    CHECK(v.capacity() == 10);
    CHECK(v.empty());
    vector<int> z(0);
    CHECK(z.capacity() == 0);
    z.push_back(5);                          // must still grow from 0
    CHECK(z.size() == 1);
    CHECK(z[0] == 5);
}

// ---------- push_back, growth, access ----------

TEST_CASE("vector: push_back one element") {
    vector<int> v;
    v.push_back(42);
    CHECK(v.size() == 1);
    CHECK_FALSE(v.empty());
    CHECK(v[0] == 42);
    CHECK(v.at(0) == 42);
}

TEST_CASE("vector: push_back past capacity doubles it and keeps every element") {
    vector<int> v;                           // capacity 2
    for (int i = 0; i < 5; ++i) v.push_back(i * 10);
    CHECK(v.size() == 5);
    CHECK(v.capacity() == 8);                // 2 -> 4 -> 8
    for (int i = 0; i < 5; ++i) CHECK(v[i] == i * 10);
}

TEST_CASE("vector: many elements") {
    vector<int> v;
    for (int i = 0; i < 10000; ++i) v.push_back(i);
    CHECK(v.size() == 10000);
    CHECK(v[0] == 0);
    CHECK(v[9999] == 9999);
}

TEST_CASE("vector: duplicates are allowed by push_back") {
    vector<int> v;
    v.push_back(7);
    v.push_back(7);
    CHECK(v.size() == 2);
}

TEST_CASE("vector: elements can be changed through [] and at") {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v[0] = 100;
    v.at(1) = 200;
    CHECK(v[0] == 100);
    CHECK(v[1] == 200);
}

TEST_CASE("vector: at throws out_of_range for a bad index") {
    vector<int> v;
    CHECK_THROWS_AS(v.at(0), std::out_of_range);   // empty
    v.push_back(1);
    CHECK_NOTHROW(v.at(0));
    CHECK_THROWS_AS(v.at(1), std::out_of_range);   // one past the end
}

TEST_CASE("vector: works with strings") {
    vector<std::string> v;
    v.push_back("ann");
    v.push_back("bob");
    v.push_back("carol");
    CHECK(v.size() == 3);
    CHECK(v[2] == "carol");
}

// ---------- pop_back and clear ----------

TEST_CASE("vector: pop_back removes the last element") {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.pop_back();
    CHECK(v.size() == 1);
    CHECK(v[0] == 1);
    v.pop_back();                            // removing the only element
    CHECK(v.empty());
}

TEST_CASE("vector: clear empties it but keeps the capacity") {
    vector<int> v;
    for (int i = 0; i < 5; ++i) v.push_back(i);
    std::size_t cap = v.capacity();
    v.clear();
    CHECK(v.empty());
    CHECK(v.capacity() == cap);
    v.push_back(9);                          // still usable after clear
    CHECK(v[0] == 9);
}

// ---------- insert_element (no duplicates) ----------

TEST_CASE("vector: insert_element adds new values and rejects duplicates") {
    vector<int> v;
    CHECK(v.insert_element(5));
    CHECK(v.insert_element(6));
    CHECK_FALSE(v.insert_element(5));        // already there
    CHECK(v.size() == 2);
}

TEST_CASE("vector: insert_element grows past capacity") {
    vector<int> v;
    for (int i = 0; i < 10; ++i) CHECK(v.insert_element(i));
    CHECK(v.size() == 10);
    CHECK(v[9] == 9);
}

// ---------- erase_element ----------

TEST_CASE("vector: erase_element") {
    vector<int> v;
    for (int i = 1; i <= 4; ++i) v.push_back(i);   // 1 2 3 4

    SUBCASE("the first element") {
        CHECK(v.erase_element(1));
        CHECK(v.size() == 3);
        CHECK(v[0] == 2);
        CHECK(v[2] == 4);
    }
    SUBCASE("a middle element keeps the order of the rest") {
        CHECK(v.erase_element(3));
        CHECK(v.size() == 3);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 4);
    }
    SUBCASE("the last element") {
        CHECK(v.erase_element(4));
        CHECK(v.size() == 3);
        CHECK(v[2] == 3);
    }
    SUBCASE("a value that is not there") {
        CHECK_FALSE(v.erase_element(99));
        CHECK(v.size() == 4);
    }
}

TEST_CASE("vector: erase_element on an empty vector and the only element") {
    vector<int> v;
    CHECK_FALSE(v.erase_element(1));
    v.push_back(1);
    CHECK(v.erase_element(1));
    CHECK(v.empty());
}

TEST_CASE("vector: erase_element removes only the first copy of a duplicate") {
    vector<int> v;
    v.push_back(7);
    v.push_back(8);
    v.push_back(7);
    CHECK(v.erase_element(7));
    CHECK(v.size() == 2);
    CHECK(v[0] == 8);
    CHECK(v[1] == 7);
}

// ---------- reserve ----------

TEST_CASE("vector: reserve grows capacity and keeps elements") {
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.reserve(100);
    CHECK(v.capacity() == 100);
    CHECK(v.size() == 2);
    CHECK(v[1] == 2);
}

TEST_CASE("vector: reserve never shrinks below the number of elements") {
    vector<int> v(10);
    for (int i = 0; i < 5; ++i) v.push_back(i);
    v.reserve(1);
    CHECK(v.capacity() == 5);
    CHECK(v.size() == 5);
    CHECK(v[4] == 4);
}

// ---------- copying and moving ----------

TEST_CASE("vector: copy constructor makes an independent copy") {
    vector<int> a;
    a.push_back(1);
    a.push_back(2);
    vector<int> b(a);
    b[0] = 99;
    CHECK(a[0] == 1);
    CHECK(b[0] == 99);
    CHECK(b.size() == 2);
}

TEST_CASE("vector: copy assignment, including to itself") {
    vector<int> a;
    a.push_back(1);
    a.push_back(2);
    vector<int> b;
    b.push_back(50);
    b = a;
    CHECK(b.size() == 2);
    CHECK(b[1] == 2);
    b[1] = 7;
    CHECK(a[1] == 2);                        // independent

    vector<int>& same = a;
    a = same;                                // self-assignment must not lose data
    CHECK(a.size() == 2);
    CHECK(a[0] == 1);
}

TEST_CASE("vector: move constructor takes the elements and empties the source") {
    vector<int> a;
    a.push_back(1);
    a.push_back(2);
    vector<int> b(std::move(a));
    CHECK(b.size() == 2);
    CHECK(b[1] == 2);
    CHECK(a.size() == 0);
    CHECK(a.capacity() == 0);
    a.push_back(5);                          // a moved-from vector is still usable
    CHECK(a[0] == 5);
}

TEST_CASE("vector: move assignment") {
    vector<int> a;
    a.push_back(1);
    vector<int> b;
    b.push_back(9);
    b.push_back(8);
    b = std::move(a);
    CHECK(b.size() == 1);
    CHECK(b[0] == 1);
    CHECK(a.empty());
}

// ---------- iteration ----------

TEST_CASE("vector: begin/end visit every element in order") {
    vector<int> v;
    for (int i = 1; i <= 3; ++i) v.push_back(i);
    int sum = 0;
    int expected = 1;
    for (int x : v) {
        CHECK(x == expected);
        ++expected;
        sum += x;
    }
    CHECK(sum == 6);
    CHECK(v.end() - v.begin() == 3);
}
