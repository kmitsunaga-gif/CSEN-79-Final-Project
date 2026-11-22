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

TEST_CASE("Inserting three keys"){
    team::BST<int,int> tree;
    
    CHECK(tree.insert(50, 100));
    CHECK(tree.insert(25,200));
    CHECK(tree.insert(12,300));
    CHECK(tree.size() == 3);
    CHECK_FALSE(tree.empty());
}

TEST_CASE("inserting keys in increasing order") {
    team::BST<int, int> tree;
    CHECK(tree.insert(1, 0));
    CHECK(tree.insert(2, 0));
    CHECK(tree.insert(3, 0));
    CHECK(tree.size() == 3);
}

TEST_CASE("inserting a duplicate key is rejected") {
    team::BST<int, int> tree;
    tree.insert(50, 0);
    tree.insert(30, 0);
    tree.insert(70, 0);
    CHECK_FALSE(tree.insert(50, 0));
    CHECK_FALSE(tree.insert(30, 0));
    CHECK_FALSE(tree.insert(70, 0));
    CHECK(tree.size() == 3);
}

TEST_CASE("inserting many keys") {
    team::BST<int, int> tree;
    for (int k = 0; k < 1000; ++k) {
        tree.insert(k, 0);
    }
    CHECK(tree.size() == 1000);
}

struct Tracked {
    static int live;
    Tracked() { ++live; }
    Tracked(const Tracked&) { ++live; }
    ~Tracked() { --live; }
};
int Tracked::live = 0;

TEST_CASE("destructor frees every node") {
    Tracked::live = 0;
    {
        team::BST<int, Tracked> tree;
        tree.insert(50, Tracked{});
        tree.insert(30, Tracked{});
        tree.insert(70, Tracked{});
        CHECK(Tracked::live == 3);
    }
    CHECK(Tracked::live == 0);
}

TEST_CASE("destroying an empty tree is safe") {
    { team::BST<int, int> tree; }
    CHECK(true);
}

TEST_CASE("find on an empty tree returns nullptr") {
    team::BST<int, int> tree;
    CHECK(tree.find(50) == nullptr);
}

TEST_CASE("find returns the value for a key that is there") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    tree.insert(30, 300);
    tree.insert(70, 700);
    REQUIRE(tree.find(50) != nullptr);
    CHECK(*tree.find(50) == 500);
    REQUIRE(tree.find(30) != nullptr);
    CHECK(*tree.find(30) == 300);
    REQUIRE(tree.find(70) != nullptr);
    CHECK(*tree.find(70) == 700);
}

TEST_CASE("find returns nullptr for a key that is not there") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    tree.insert(30, 300);
    tree.insert(70, 700);
    CHECK(tree.find(10) == nullptr);
    CHECK(tree.find(40) == nullptr);
    CHECK(tree.find(99) == nullptr);
}

TEST_CASE("a duplicate insert does not overwrite the value") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    tree.insert(50, 999);
    REQUIRE(tree.find(50) != nullptr);
    CHECK(*tree.find(50) == 500);
}

TEST_CASE("the value can be changed through find") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    int* value = tree.find(50);
    REQUIRE(value != nullptr);
    *value = 123;
    CHECK(*tree.find(50) == 123);
}