// Tests for CSEN79_StockBook::Order (src/order.h, src/order.cpp).
// An Order holds one order's id, price, share quantity, and user.
#include <sstream>
#include <string>

#include "doctest/doctest.h"
#include "order.h"

using CSEN79_StockBook::Order;

// ---------- constructor and getters ----------

TEST_CASE("Order: constructor stores every field") {
    Order o(7, 50.25, 100, "bob");
    CHECK(o.get_id() == 7);
    CHECK(o.get_price() == doctest::Approx(50.25));
    CHECK(o.get_shares() == 100);
    CHECK(o.get_user() == "bob");
    CHECK_FALSE(o.isFilled());
}

TEST_CASE("Order: a market order uses price 0.00") {
    Order o(8, 0.00, 500, "alice");
    CHECK(o.get_price() == 0.00);
    CHECK(o.get_shares() == 500);
}

TEST_CASE("Order: an order with one share") {
    Order o(1, 10.00, 1, "ann");
    CHECK(o.get_shares() == 1);
    CHECK_FALSE(o.isFilled());
}

// ---------- copy constructor ----------

TEST_CASE("Order: copy has the same fields") {
    Order original(3, 49.99, 250, "carol");
    Order copy(original);
    CHECK(copy.get_id() == 3);
    CHECK(copy.get_price() == doctest::Approx(49.99));
    CHECK(copy.get_shares() == 250);
    CHECK(copy.get_user() == "carol");
}

TEST_CASE("Order: changing the copy does not change the original") {
    Order original(3, 49.99, 250, "carol");
    Order copy(original);
    copy.setShares(10);
    CHECK(copy.get_shares() == 10);
    CHECK(original.get_shares() == 250);
}

// ---------- setShares ----------

TEST_CASE("Order: setShares replaces the share count") {
    Order o(1, 50.00, 100, "dave");
    o.setShares(40);
    CHECK(o.get_shares() == 40);
    o.setShares(0);
    CHECK(o.get_shares() == 0);
    CHECK(o.isFilled());
}

// ---------- reduceShares (fills) ----------

TEST_CASE("Order: partial fill leaves the rest") {
    Order o(1, 50.00, 100, "erin");
    CHECK(o.reduceShares(30) == 0);        // order covers the whole fill, nothing unfilled
    CHECK(o.get_shares() == 70);
    CHECK_FALSE(o.isFilled());
}

TEST_CASE("Order: exact fill empties the order") {
    Order o(1, 50.00, 100, "erin");
    CHECK(o.reduceShares(100) == 0);
    CHECK(o.get_shares() == 0);
    CHECK(o.isFilled());
}

TEST_CASE("Order: a fill larger than the order returns the unfilled part") {
    Order o(1, 50.00, 100, "erin");
    CHECK(o.reduceShares(130) == 30);       // 100 filled here, 30 left over
    CHECK(o.get_shares() == 0);
    CHECK(o.isFilled());
}

TEST_CASE("Order: several fills in a row") {
    Order o(1, 50.00, 100, "frank");
    o.reduceShares(25);
    o.reduceShares(25);
    CHECK(o.get_shares() == 50);
    o.reduceShares(50);
    CHECK(o.isFilled());
}

TEST_CASE("Order: one-share order filled by one share") {
    Order o(1, 50.00, 1, "gina");
    CHECK(o.reduceShares(1) == 0);
    CHECK(o.isFilled());
}

// ---------- operator<< (debug output) ----------

TEST_CASE("Order: operator<< prints user, id, price and shares") {
    Order o(4, 50.5, 100, "dave");
    std::ostringstream out;
    out << o;
    CHECK(out.str() == "dave [ID=4 @ $50.5 x 100]");
}
