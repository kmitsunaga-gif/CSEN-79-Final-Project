// Tests for the order book running on STL containers: stock_book<StdTraits> (src/stock_book.h).
// Each test calls the book's functions directly and compares what it prints with the exact
// format in docs/OUTPUT_FORMAT.md.
//
// Tests marked "* doctest::may_fail()" check a rule the code does not follow yet (a known bug).
// They are reported but do not stop the suite. Remove may_fail once the bug is fixed, so the
// test guards the fix from then on.
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "doctest/doctest.h"
#include "stock_book.h"

using namespace CSEN79_StockBook;

namespace {

using Book = stock_book<StdTraits>;

// Runs f() and returns everything it printed to std::cout. Prices print with two decimals,
// the same way main.cpp sets up std::cout.
template <typename F>
std::string capture(F f) {
    std::ostringstream out;
    std::streambuf* old_buf = std::cout.rdbuf(out.rdbuf());
    std::ios::fmtflags old_flags = std::cout.flags();
    std::streamsize old_precision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(2);
    f();
    std::cout.rdbuf(old_buf);
    std::cout.flags(old_flags);
    std::cout.precision(old_precision);
    return out.str();
}

// The book from the start of samples/p1_demo.txt (commands #1 to #6):
//   asks: 50.20 bob 200, carol 150   |   50.30 alice 100
//   bids: 50.10 erin 300, frank 50   |   50.00 dave 100
void build_sample_book(Book& book) {
    capture([&] {
        book.sellLimit("alice", 1, 100, 50.30, 1);
        book.sellLimit("bob", 2, 200, 50.20, 2);
        book.sellLimit("carol", 3, 150, 50.20, 3);
        book.buyLimit("dave", 4, 100, 50.00, 4);
        book.buyLimit("erin", 5, 300, 50.10, 5);
        book.buyLimit("frank", 6, 50, 50.10, 6);
    });
}

}  // namespace

// ---------- limit orders that rest ----------

TEST_CASE("STL book: a limit order with nothing to match rests") {
    Book book;
    CHECK(capture([&] { book.buyLimit("dave", 4, 100, 50.00, 1); }) == "REST 4 100 50.00\n");
    CHECK(capture([&] { book.sellLimit("alice", 1, 100, 50.30, 2); }) == "REST 1 100 50.30\n");
}

TEST_CASE("STL book: orders that do not cross both rest") {
    Book book;
    capture([&] { book.buyLimit("dave", 4, 100, 50.00, 1); });
    CHECK(capture([&] { book.sellLimit("alice", 1, 100, 50.10, 2); }) == "REST 1 100 50.10\n");
}

// ---------- matching ----------

TEST_CASE("STL book: a buy that crosses fills the oldest order at the best price first") {
    Book book;
    build_sample_book(book);
    // command #8 in the sample: fills bob (older), then carol, then rests the rest
    CHECK(capture([&] { book.buyLimit("gina", 7, 400, 50.20, 8); }) ==
          "TRADE 7 2 200 50.20\n"
          "TRADE 7 3 150 50.20\n"
          "REST 7 50 50.20\n");
}

TEST_CASE("STL book: trades happen at the resting order's price") {
    Book book;
    capture([&] { book.sellLimit("ann", 1, 100, 50.00, 1); });
    // buyer would pay up to 51.00, but the trade is at the seller's 50.00
    CHECK(capture([&] { book.buyLimit("bob", 2, 100, 51.00, 2); }) == "TRADE 2 1 100 50.00\n");
}

TEST_CASE("STL book: a sell matches the highest bid first, buy id printed first") {
    Book book;
    capture([&] {
        book.buyLimit("ann", 1, 100, 50.00, 1);
        book.buyLimit("bob", 2, 100, 50.10, 2);
    });
    CHECK(capture([&] { book.sellLimit("carol", 3, 150, 50.00, 3); }) ==
          "TRADE 2 3 100 50.10\n"
          "TRADE 1 3 50 50.00\n");
}

TEST_CASE("STL book: a limit buy stops at its price and rests the remainder") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 50, 50.00, 1);
        book.sellLimit("bob", 2, 50, 50.50, 2);
    });
    CHECK(capture([&] { book.buyLimit("carol", 3, 100, 50.20, 3); }) ==
          "TRADE 3 1 50 50.00\n"
          "REST 3 50 50.20\n");
}

TEST_CASE("STL book: an exact fill prints no REST") {
    Book book;
    capture([&] { book.sellLimit("ann", 1, 100, 50.00, 1); });
    CHECK(capture([&] { book.buyLimit("bob", 2, 100, 50.00, 2); }) == "TRADE 2 1 100 50.00\n");
}

TEST_CASE("STL book: a partial fill leaves the resting order with fewer shares") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.00, 1);
        book.buyLimit("bob", 2, 30, 50.00, 2);
    });
    // ann's order still has 70, so a second buyer gets them
    CHECK(capture([&] { book.buyLimit("carol", 3, 100, 50.00, 3); }) ==
          "TRADE 3 1 70 50.00\n"
          "REST 3 30 50.00\n");
}

// ---------- market orders ----------

TEST_CASE("STL book: a market sell sweeps the bids, best price first, then UNFILLED") {
    Book book;
    build_sample_book(book);
    capture([&] { book.buyLimit("gina", 7, 400, 50.20, 8); });
    capture([&] { book.cancelOrder(6); });
    // command #12 in the sample
    CHECK(capture([&] { book.sellMarket("alice", 8, 500, 12); }) ==
          "TRADE 7 8 50 50.20\n"
          "TRADE 5 8 300 50.10\n"
          "TRADE 4 8 100 50.00\n"
          "UNFILLED 8 50\n");
}

TEST_CASE("STL book: a market order on an empty side is all UNFILLED") {
    Book book;
    CHECK(capture([&] { book.buyMarket("dave", 9, 1000, 1); }) == "UNFILLED 9 1000\n");
}

TEST_CASE("STL book: a market order never rests") {
    Book book;
    capture([&] { book.buyMarket("dave", 9, 100, 1); });
    CHECK(capture([&] { book.bookPrint(5); }) == "END\n");
}

TEST_CASE("STL book: a fully filled market order prints no UNFILLED" * doctest::may_fail()) {
    // known bug: prints "UNFILLED 2 0"
    Book book;
    capture([&] { book.sellLimit("ann", 1, 100, 50.00, 1); });
    CHECK(capture([&] { book.buyMarket("bob", 2, 100, 2); }) == "TRADE 2 1 100 50.00\n");
}

// ---------- ids ----------

TEST_CASE("STL book: reusing an id is an error and changes nothing") {
    Book book;
    capture([&] { book.buyLimit("erin", 5, 300, 50.10, 1); });
    CHECK(capture([&] { book.buyLimit("erin", 5, 10, 49.00, 2); }) == "ERROR duplicate id 5\n");
    CHECK(capture([&] { book.sellLimit("ann", 5, 10, 60.00, 3); }) == "ERROR duplicate id 5\n");
    CHECK(capture([&] { book.buyMarket("ann", 5, 10, 4); }) == "ERROR duplicate id 5\n");
}

TEST_CASE("STL book: ids of filled and market orders cannot be reused") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.00, 1);
        book.buyLimit("bob", 2, 100, 50.00, 2);     // fills order 1 completely
        book.buyMarket("carol", 3, 10, 3);          // market order 3, nothing to fill
    });
    CHECK(capture([&] { book.sellLimit("dave", 1, 5, 51.00, 4); }) == "ERROR duplicate id 1\n");
    CHECK(capture([&] { book.sellLimit("dave", 3, 5, 51.00, 5); }) == "ERROR duplicate id 3\n");
}

// ---------- cancel ----------

TEST_CASE("STL book: cancel a resting bid and a resting ask") {
    Book book;
    build_sample_book(book);
    CHECK(capture([&] { book.cancelOrder(6); }) == "CANCELLED 6 50\n");
    CHECK(capture([&] { book.cancelOrder(1); }) == "CANCELLED 1 100\n");
}

TEST_CASE("STL book: cancel reports the remaining quantity after a partial fill") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.00, 1);
        book.buyLimit("bob", 2, 40, 50.00, 2);
    });
    CHECK(capture([&] { book.cancelOrder(1); }) == "CANCELLED 1 60\n");
}

TEST_CASE("STL book: cancel an id that never existed") {
    Book book;
    CHECK(capture([&] { book.cancelOrder(42); }) == "ERROR unknown order 42\n");
}

TEST_CASE("STL book: cancelling the same order twice" * doctest::may_fail()) {
    // known bug: the second cancel prints nothing
    Book book;
    build_sample_book(book);
    capture([&] { book.cancelOrder(6); });
    CHECK(capture([&] { book.cancelOrder(6); }) == "ERROR unknown order 6\n");
}

TEST_CASE("STL book: cancelling a filled order" * doctest::may_fail()) {
    // known bug: prints nothing
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.00, 1);
        book.buyLimit("bob", 2, 100, 50.00, 2);
    });
    CHECK(capture([&] { book.cancelOrder(1); }) == "ERROR unknown order 1\n");
}

TEST_CASE("STL book: cancelling the only order at a price removes the level") {
    Book book;
    capture([&] { book.sellLimit("ann", 1, 100, 50.00, 1); });
    capture([&] { book.cancelOrder(1); });
    // a buy at 50.00 now has nothing to match
    CHECK(capture([&] { book.buyLimit("bob", 2, 10, 50.00, 3); }) == "REST 2 10 50.00\n");
}

TEST_CASE("STL book: a cancelled order is skipped by later matching") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.00, 1);
        book.sellLimit("bob", 2, 100, 50.00, 2);
        book.cancelOrder(1);
    });
    CHECK(capture([&] { book.buyLimit("carol", 3, 50, 50.00, 4); }) == "TRADE 3 2 50 50.00\n");
}

// ---------- BOOK ----------

TEST_CASE("STL book: BOOK on an empty book prints only END") {
    Book book;
    CHECK(capture([&] { book.bookPrint(5); }) == "END\n");
}

TEST_CASE("STL book: BOOK with only asks does not crash") {
    Book book;
    capture([&] { book.sellLimit("ann", 1, 100, 50.30, 1); });
    std::string out = capture([&] { book.bookPrint(5); });
    CHECK(out.find("END") != std::string::npos);
}

TEST_CASE("STL book: BOOK lists asks lowest first, then bids highest first" * doctest::may_fail()) {
    // known bug: lines are missing the ASK / BID label
    Book book;
    build_sample_book(book);
    CHECK(capture([&] { book.bookPrint(5); }) ==
          "ASK 50.20 350 2\n"
          "ASK 50.30 100 1\n"
          "BID 50.10 350 2\n"
          "BID 50.00 100 1\n"
          "END\n");
}

TEST_CASE("STL book: BOOK depth limits each side" * doctest::may_fail()) {
    // known bug: lines are missing the ASK / BID label
    Book book;
    build_sample_book(book);
    CHECK(capture([&] { book.bookPrint(1); }) ==
          "ASK 50.20 350 2\n"
          "BID 50.10 350 2\n"
          "END\n");
}

TEST_CASE("STL book: BOOK totals and counts follow fills and cancels") {
    Book book;
    build_sample_book(book);
    capture([&] {
        book.buyLimit("gina", 7, 400, 50.20, 8);   // clears the 50.20 asks, rests a 50.20 bid
        book.cancelOrder(6);                       // frank leaves 50.10
    });
    std::string out = capture([&] { book.bookPrint(3); });
    // checked without the ASK/BID labels, so this test is not blocked by that bug
    CHECK(out.find("50.30 100 1\n") != std::string::npos);
    CHECK(out.find("50.20 50 1\n") != std::string::npos);
    CHECK(out.find("50.10 300 1\n") != std::string::npos);
    CHECK(out.find("50.00 100 1\n") != std::string::npos);
}

// ---------- HISTORY ----------

TEST_CASE("STL book: HISTORY of an unknown user prints only END") {
    Book book;
    CHECK(capture([&] { book.historyPrint("nobody"); }) == "END\n");
}

TEST_CASE("STL book: HISTORY lists places and fills for both sides, oldest first") {
    Book book;
    build_sample_book(book);
    capture([&] { book.buyLimit("gina", 7, 400, 50.20, 8); });
    CHECK(capture([&] { book.historyPrint("gina"); }) ==
          "#8 PLACE 7 BUY 400 50.20\n"
          "#8 FILL 7 200 50.20\n"
          "#8 FILL 7 150 50.20\n"
          "END\n");
    CHECK(capture([&] { book.historyPrint("bob"); }) ==
          "#2 PLACE 2 SELL 200 50.20\n"
          "#8 FILL 2 200 50.20\n"
          "END\n");
}

TEST_CASE("STL book: HISTORY shows MKT for market orders and UNFILLED") {
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.30, 1);
        book.buyMarket("dave", 9, 1000, 2);
    });
    CHECK(capture([&] { book.historyPrint("dave"); }) ==
          "#2 PLACE 9 BUY 1000 MKT\n"
          "#2 FILL 9 100 50.30\n"
          "#2 UNFILLED 9 900\n"
          "END\n");
}

TEST_CASE("STL book: HISTORY with n shows only the last n events" * doctest::may_fail()) {
    // known bug: shows the first n events instead
    Book book;
    capture([&] {
        book.sellLimit("ann", 1, 100, 50.30, 1);
        book.buyMarket("dave", 9, 1000, 2);
    });
    CHECK(capture([&] { book.historyPrint("dave", 2); }) ==
          "#2 FILL 9 100 50.30\n"
          "#2 UNFILLED 9 900\n"
          "END\n");
}

TEST_CASE("STL book: HISTORY with n larger than the history shows everything") {
    Book book;
    capture([&] { book.buyLimit("dave", 4, 100, 50.00, 1); });
    CHECK(capture([&] { book.historyPrint("dave", 10); }) ==
          "#1 PLACE 4 BUY 100 50.00\n"
          "END\n");
}

// ---------- the whole sample, command by command ----------

TEST_CASE("STL book: samples/p1_demo.txt, run through the book directly") {
    Book book;
    std::string out = capture([&] {
        book.sellLimit("alice", 1, 100, 50.30, 1);
        book.sellLimit("bob", 2, 200, 50.20, 2);
        book.sellLimit("carol", 3, 150, 50.20, 3);
        book.buyLimit("dave", 4, 100, 50.00, 4);
        book.buyLimit("erin", 5, 300, 50.10, 5);
        book.buyLimit("frank", 6, 50, 50.10, 6);
        book.buyLimit("gina", 7, 400, 50.20, 8);
        book.cancelOrder(6);
        book.sellMarket("alice", 8, 500, 12);
        book.buyMarket("dave", 9, 1000, 14);
        book.buyLimit("erin", 5, 10, 49.00, 15);
    });
    CHECK(out ==
          "REST 1 100 50.30\n"
          "REST 2 200 50.20\n"
          "REST 3 150 50.20\n"
          "REST 4 100 50.00\n"
          "REST 5 300 50.10\n"
          "REST 6 50 50.10\n"
          "TRADE 7 2 200 50.20\n"
          "TRADE 7 3 150 50.20\n"
          "REST 7 50 50.20\n"
          "CANCELLED 6 50\n"
          "TRADE 7 8 50 50.20\n"
          "TRADE 5 8 300 50.10\n"
          "TRADE 4 8 100 50.00\n"
          "UNFILLED 8 50\n"
          "TRADE 9 1 100 50.30\n"
          "UNFILLED 9 900\n"
          "ERROR duplicate id 5\n");
    CHECK(capture([&] { book.historyPrint("alice"); }) ==
          "#1 PLACE 1 SELL 100 50.30\n"
          "#12 PLACE 8 SELL 500 MKT\n"
          "#12 FILL 8 50 50.20\n"
          "#12 FILL 8 300 50.10\n"
          "#12 FILL 8 100 50.00\n"
          "#12 UNFILLED 8 50\n"
          "#14 FILL 1 100 50.30\n"
          "END\n");
}
