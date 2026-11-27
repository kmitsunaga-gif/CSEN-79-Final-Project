// Tests for CSEN79_StockBook::TailForwardList (in src/stock_book.h), the per-user HISTORY list.
// It wraps a forward list and keeps a pointer to the last element so push_back is O(1).
// These tests use the STL forward list (std::forward_list), the version StdTraits uses.
#include <forward_list>
#include <utility>

#include "doctest/doctest.h"
#include "stock_book.h"

using namespace CSEN79_StockBook;

namespace {

using Event = std::pair<Order*, historyTag>;
using History = TailForwardList<std::forward_list<Event>>;

// Collects the command numbers of every event, in list order, into out. Returns how many.
std::size_t numbers(const History& h, std::size_t* out) {
    std::size_t n = 0;
    for (auto it = h.begin(); it != h.end(); ++it) out[n++] = it->second.num;
    return n;
}

}  // namespace

TEST_CASE("TailForwardList: a new list is empty") {
    History h;
    CHECK(h.empty());
    CHECK(h.count == 0);
    CHECK(h.size() == 0);
    CHECK(h.begin() == h.end());
}

TEST_CASE("TailForwardList: push_back one event") {
    Order o(1, 50.00, 100, "ann");
    History h;
    h.push_back({&o, historyTag{'b', 1}});
    CHECK_FALSE(h.empty());
    CHECK(h.count == 1);
    CHECK(h.begin()->first == &o);
    CHECK(h.begin()->second.action == 'b');
    CHECK(h.begin()->second.num == 1);
}

TEST_CASE("TailForwardList: push_back keeps events oldest first") {
    Order o(1, 50.00, 100, "ann");
    History h;
    for (std::size_t i = 1; i <= 5; ++i) h.push_back({&o, historyTag{'f', i}});
    std::size_t got[5];
    REQUIRE(numbers(h, got) == 5);
    for (std::size_t i = 0; i < 5; ++i) CHECK(got[i] == i + 1);
    CHECK(h.count == 5);
    CHECK(h.size() == 5);
}

TEST_CASE("TailForwardList: push_front onto an empty list, then push_back") {
    Order o(1, 50.00, 100, "ann");
    History h;
    h.push_front({&o, historyTag{'b', 1}});
    h.push_back({&o, historyTag{'f', 2}});
    std::size_t got[2];
    REQUIRE(numbers(h, got) == 2);
    CHECK(got[0] == 1);
    CHECK(got[1] == 2);
}

TEST_CASE("TailForwardList: push_front adds at the start and push_back still adds at the end") {
    Order o(1, 50.00, 100, "ann");
    History h;
    h.push_back({&o, historyTag{'b', 2}});
    h.push_front({&o, historyTag{'b', 1}});   // new first element; the tail must not move
    h.push_back({&o, historyTag{'f', 3}});
    std::size_t got[3];
    REQUIRE(numbers(h, got) == 3);
    CHECK(got[0] == 1);
    CHECK(got[1] == 2);
    CHECK(got[2] == 3);
    CHECK(h.count == 3);
    CHECK(h.size() == 3);                     // size() counts push_front and push_back
}

TEST_CASE("TailForwardList: stores the history tag's action letters unchanged") {
    Order o(1, 50.00, 100, "ann");
    History h;
    const char actions[4] = {'b', 's', 'f', 'u'};
    for (std::size_t i = 0; i < 4; ++i) h.push_back({&o, historyTag{actions[i], i + 1}});
    std::size_t i = 0;
    for (auto it = h.begin(); it != h.end(); ++it, ++i) CHECK(it->second.action == actions[i]);
}

TEST_CASE("TailForwardList: many events") {
    Order o(1, 50.00, 100, "ann");
    History h;
    for (std::size_t i = 1; i <= 10000; ++i) h.push_back({&o, historyTag{'f', i}});
    CHECK(h.count == 10000);
    std::size_t last = 0;
    for (auto it = h.begin(); it != h.end(); ++it) last = it->second.num;
    CHECK(last == 10000);
}
