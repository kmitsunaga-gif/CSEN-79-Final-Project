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

TEST_CASE("Inserting a key"){
    team::BST<int,int> tree;
    bool added = tree.insert(50, 100);
    CHECK(added);
    CHECK(tree.size() == 1);
    CHECK_FALSE(tree.empty());
}

TEST_CASE("Inserting three keyes"){
    team::BST<int,int> tree;
    
    CHECK(tree.insert(50, 100));
    CHECK(tree.insert(25,200));
    CHECK(tree.insert(12,300));
    CHECK(tree.size() == 3);
    CHECK_FALSE(tree.empty());
}