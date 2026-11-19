#include "doctest/doctest.h"
#include "team/bst.h"

TEST_CASE("New Tree is Empty"){
    team::BST<int, int> tree;
    SUBCASE("Checks size starts at 0"){
        CHECK(tree.size() == 0);
    }
    SUBCASE("Empty works appropriately"){
        CHECK(tree.empty() == true);
    }
}