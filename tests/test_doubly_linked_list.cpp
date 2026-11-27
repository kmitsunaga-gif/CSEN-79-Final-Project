// Tests for CSEN79_StockBook::doubly_linked_list (src/doubly_linked_list.h), the hand-built
// linear structure: the FIFO line of orders at one price level.
// Covers the PDF's required edge cases (empty, one element, duplicates, removing the first,
// last and only element), FIFO order, iterators (including -- from end()), copying,
// memory, and the ListMetrics counters.
#include <string>
#include <type_traits>

#include "doctest/doctest.h"
#include "doubly_linked_list.h"
#include "order.h"

using CSEN79_StockBook::doubly_linked_list;
using CSEN79_StockBook::Order;

namespace {

using List = doubly_linked_list<int>;

// Counts how many copies of itself exist, to check every node is freed.
struct TrackedItem {
    static int live;
    int value;
    TrackedItem(int v = 0) : value(v) { ++live; }
    TrackedItem(const TrackedItem& o) : value(o.value) { ++live; }
    TrackedItem& operator=(const TrackedItem& o) { value = o.value; return *this; }
    ~TrackedItem() { --live; }
    bool operator==(const TrackedItem& o) const { return value == o.value; }
};
int TrackedItem::live = 0;

// Copies the list front to back into out; returns how many.
int contents(const List& l, int* out) {
    int n = 0;
    for (auto it = l.begin(); it != l.end(); ++it) out[n++] = *it;
    return n;
}

List make_list(int count) {           // 1, 2, ..., count
    List l;
    for (int i = 1; i <= count; ++i) l.push_back(i);
    return l;
}

}  // namespace

// ---------- empty list ----------

TEST_CASE("list: a new list is empty") {
    List l;
    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());
    CHECK_FALSE(l.remove(5));
}

TEST_CASE("list: clear on an empty list is safe, twice") {
    List l;
    l.clear();
    l.clear();
    CHECK(l.empty());
}

// ---------- one element ----------

TEST_CASE("list: one element") {
    List l;
    l.push_back(7);
    CHECK_FALSE(l.empty());
    CHECK(l.size() == 1);
    CHECK(l.front() == 7);
    CHECK(*l.begin() == 7);
}

TEST_CASE("list: the one-value constructor") {
    List l(9);
    CHECK(l.size() == 1);
    CHECK(l.front() == 9);
}

TEST_CASE("list: pop_front of the only element empties it") {
    List l;
    l.push_back(7);
    l.pop_front();
    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());
    l.push_back(8);                         // usable again: head and tail were reset
    CHECK(l.front() == 8);
    CHECK(l.size() == 1);
}

TEST_CASE("list: remove the only element empties it") {
    List l;
    l.push_back(7);
    CHECK(l.remove(7));
    CHECK(l.empty());
    l.push_back(8);
    CHECK(l.front() == 8);
}

// ---------- FIFO order (the time priority at a price level) ----------

TEST_CASE("list: push_back adds at the back, front is the oldest") {
    List l = make_list(3);
    CHECK(l.front() == 1);
    int got[3];
    REQUIRE(contents(l, got) == 3);
    CHECK(got[0] == 1);
    CHECK(got[1] == 2);
    CHECK(got[2] == 3);
}

TEST_CASE("list: pop_front removes the oldest first") {
    List l = make_list(3);
    l.pop_front();
    CHECK(l.front() == 2);
    CHECK(l.size() == 2);
    l.pop_front();
    CHECK(l.front() == 3);
    l.pop_front();
    CHECK(l.empty());
}

TEST_CASE("list: interleaved push_back and pop_front keep FIFO order") {
    List l;
    l.push_back(1);
    l.push_back(2);
    l.pop_front();                          // 1 leaves
    l.push_back(3);
    CHECK(l.front() == 2);
    l.pop_front();
    CHECK(l.front() == 3);
}

TEST_CASE("list: many elements") {
    List l;
    for (int i = 0; i < 10000; ++i) l.push_back(i);
    CHECK(l.size() == 10000);
    for (int i = 0; i < 10000; ++i) {
        REQUIRE(l.front() == i);
        l.pop_front();
    }
    CHECK(l.empty());
}

TEST_CASE("list: front can be changed in place") {
    List l = make_list(2);
    l.front() = 100;
    CHECK(*l.begin() == 100);
}

// ---------- remove: first, middle, last, missing, duplicates ----------

TEST_CASE("list: remove") {
    List l = make_list(4);                  // 1 2 3 4
    int got[4];

    SUBCASE("the first element") {
        CHECK(l.remove(1));
        REQUIRE(contents(l, got) == 3);
        CHECK(got[0] == 2);
        CHECK(l.front() == 2);
    }
    SUBCASE("a middle element keeps the others in order") {
        CHECK(l.remove(3));
        REQUIRE(contents(l, got) == 3);
        CHECK(got[0] == 1);
        CHECK(got[1] == 2);
        CHECK(got[2] == 4);
    }
    SUBCASE("the last element, then push_back links to the new tail") {
        CHECK(l.remove(4));
        l.push_back(5);
        REQUIRE(contents(l, got) == 4);
        CHECK(got[2] == 3);
        CHECK(got[3] == 5);
    }
    SUBCASE("a value that is not there") {
        CHECK_FALSE(l.remove(99));
        CHECK(l.size() == 4);
    }
    SUBCASE("every element, from the back") {
        for (int v = 4; v >= 1; --v) CHECK(l.remove(v));
        CHECK(l.empty());
    }
}

TEST_CASE("list: remove takes only the first of duplicate values") {
    List l;
    l.push_back(7);
    l.push_back(8);
    l.push_back(7);
    CHECK(l.remove(7));
    int got[2];
    REQUIRE(contents(l, got) == 2);
    CHECK(got[0] == 8);
    CHECK(got[1] == 7);
}

TEST_CASE("list: size follows push, pop and remove") {
    List l = make_list(5);
    l.pop_front();
    l.remove(3);
    l.remove(99);                           // not there: size unchanged
    CHECK(l.size() == 3);
}

TEST_CASE("list: holds Order* and removes by pointer, as a price level does") {
    Order a(1, 50.00, 100, "ann");
    Order b(2, 50.00, 200, "bob");
    Order c(3, 50.00, 150, "carol");
    doubly_linked_list<Order*> level;
    level.push_back(&a);
    level.push_back(&b);
    level.push_back(&c);
    CHECK(level.remove(&b));                // cancel the middle order
    CHECK(level.front() == &a);             // oldest is still first
    level.pop_front();                      // a fills
    CHECK(level.front() == &c);
    CHECK(level.size() == 1);
}

// ---------- iterators ----------

TEST_CASE("list: postfix ++ returns the old position") {
    List l = make_list(2);
    auto it = l.begin();
    auto old = it++;
    CHECK(*old == 1);
    CHECK(*it == 2);
}

TEST_CASE("list: values can be changed through an iterator") {
    List l = make_list(3);
    for (auto it = l.begin(); it != l.end(); ++it) *it *= 10;
    int got[3];
    REQUIRE(contents(l, got) == 3);
    CHECK(got[2] == 30);
}

TEST_CASE("list: -- from end() reaches the last element") {
    List l = make_list(3);
    auto it = l.end();
    --it;
    CHECK(*it == 3);
    --it;
    CHECK(*it == 2);
    auto old = it--;                        // postfix
    CHECK(*old == 2);
    CHECK(*it == 1);
}

TEST_CASE("list: -- from end() on a const list (const_iterator)") {
    List l = make_list(3);
    const List& c = l;
    auto it = c.end();
    --it;
    CHECK(*it == 3);
    auto old = it--;
    CHECK(*old == 3);
    CHECK(*it == 2);
}

TEST_CASE("list: -- from end() after removing the tail reaches the new tail") {
    List l = make_list(3);
    l.remove(3);
    auto it = l.end();
    --it;
    CHECK(*it == 2);
}

TEST_CASE("list: iterator converts to const_iterator") {
    List l = make_list(3);
    List::const_iterator ci = l.begin();
    CHECK(*ci == 1);
    List::const_iterator ce = l.end();
    --ce;                                   // the converted end() still knows its list
    CHECK(*ce == 3);
}

TEST_CASE("list: a const list can be walked with const_iterator") {
    List l = make_list(4);
    const List& c = l;
    int sum = 0;
    for (auto it = c.begin(); it != c.end(); ++it) sum += *it;
    CHECK(sum == 10);
    CHECK(c.front() == 1);
}

// ---------- copying ----------

TEST_CASE("list: copy constructor makes an independent copy") {
    List a = make_list(3);
    List b(a);
    b.front() = 100;
    b.push_back(4);
    CHECK(a.front() == 1);
    CHECK(a.size() == 3);
    CHECK(b.size() == 4);
}

TEST_CASE("list: copying an empty list") {
    List a;
    List b(a);
    CHECK(b.empty());
}

TEST_CASE("list: assignment replaces the contents, including to itself") {
    List a = make_list(3);
    List b = make_list(5);
    b = a;
    CHECK(b.size() == 3);
    int got[3];
    REQUIRE(contents(b, got) == 3);
    CHECK(got[2] == 3);
    b.pop_front();
    CHECK(a.size() == 3);                   // independent

    List& same = a;
    a = same;                               // self-assignment keeps the data
    CHECK(a.size() == 3);
    CHECK(a.front() == 1);
}

// ---------- memory ----------

TEST_CASE("list: destructor, pop_front, remove and clear free every node") {
    TrackedItem::live = 0;
    {
        doubly_linked_list<TrackedItem> l;
        for (int i = 0; i < 10; ++i) l.push_back(TrackedItem(i));
        CHECK(TrackedItem::live == 10);
        l.pop_front();
        l.remove(TrackedItem(5));
        CHECK(TrackedItem::live == 8);
        l.clear();
        CHECK(TrackedItem::live == 0);
        for (int i = 0; i < 5; ++i) l.push_back(TrackedItem(i));
    }                                       // destructor frees the last 5
    CHECK(TrackedItem::live == 0);
}

// ---------- counters (ListMetrics) ----------

TEST_CASE("list: counters count allocations and deallocations exactly") {
    List l;
    for (int i = 1; i <= 5; ++i) l.push_back(i);
    CHECK(l.metrics().allocations == 5);
    l.pop_front();
    l.remove(3);
    CHECK(l.metrics().deallocations == 2);
    l.clear();                              // 3 left
    CHECK(l.metrics().deallocations == 5);
}

TEST_CASE("list: remove visits exactly the nodes up to the target") {
    List l = make_list(5);
    l.resetMetrics();
    l.remove(3);                            // visits 1, 2, 3
    CHECK(l.metrics().nodesVisited == 3);
    CHECK(l.metrics().comparisons == 3);

    l.resetMetrics();
    l.remove(99);                           // not there: visits all 4 left
    CHECK(l.metrics().nodesVisited == 4);
    CHECK(l.metrics().comparisons == 4);

    l.resetMetrics();
    l.remove(1);                            // the front: 1 visit
    CHECK(l.metrics().nodesVisited == 1);
}

TEST_CASE("list: resetMetrics sets every counter to 0") {
    List l = make_list(3);
    l.remove(2);
    l.resetMetrics();
    CHECK(l.metrics().allocations == 0);
    CHECK(l.metrics().deallocations == 0);
    CHECK(l.metrics().nodesVisited == 0);
    CHECK(l.metrics().comparisons == 0);
}
