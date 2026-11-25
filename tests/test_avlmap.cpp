#include "doctest/doctest.h"          // match whatever test_kit.cpp uses
#include "../src/AvlMap.h"              // adjust to your real path

using namespace final_project_map;

TEST_CASE("AvlMap basic insert and invariants") {
    AvlMap<int, int> m;
    CHECK(m.check_invariants());          // empty tree
    for (int i = 0; i < 1000; ++i) {
        m[i] = i;                         // sorted input
        CHECK(m.check_invariants());
    }
    CHECK(m.find(500));
    CHECK(m.erase(500));
    CHECK_FALSE(m.find(500));
    CHECK(m.check_invariants());
}