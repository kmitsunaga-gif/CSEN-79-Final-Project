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

TEST_CASE("min and max on an empty tree return nullptrs") {
    team::BST<int, int> tree;
    CHECK(tree.min().key == nullptr);
    CHECK(tree.min().value == nullptr);
    CHECK(tree.max().key == nullptr);
    CHECK(tree.max().value == nullptr);
}

TEST_CASE("min and max of a one-node tree are the same node") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    REQUIRE(tree.min().key != nullptr);
    REQUIRE(tree.max().key != nullptr);
    CHECK(*tree.min().key == 50);
    CHECK(*tree.max().key == 50);
    CHECK(*tree.min().value == 500);
    CHECK(*tree.max().value == 500);
}

TEST_CASE("min and max of a mixed tree") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    tree.insert(30, 300);
    tree.insert(70, 700);
    tree.insert(20, 200);
    tree.insert(80, 800);
    tree.insert(40, 400);
    tree.insert(60, 600);
    REQUIRE(tree.min().key != nullptr);
    REQUIRE(tree.max().key != nullptr);
    CHECK(*tree.min().key == 20);
    CHECK(*tree.min().value == 200);
    CHECK(*tree.max().key == 80);
    CHECK(*tree.max().value == 800);
}

TEST_CASE("min is the root when nothing is smaller") {
    team::BST<int, int> tree;
    tree.insert(10, 0);
    tree.insert(20, 0);
    tree.insert(30, 0);
    REQUIRE(tree.min().key != nullptr);
    CHECK(*tree.min().key == 10);
    CHECK(*tree.max().key == 30);
}

TEST_CASE("max is the root when nothing is larger") {
    team::BST<int, int> tree;
    tree.insert(30, 0);
    tree.insert(20, 0);
    tree.insert(10, 0);
    REQUIRE(tree.max().key != nullptr);
    CHECK(*tree.max().key == 30);
    CHECK(*tree.min().key == 10);
}

TEST_CASE("min and max follow new inserts") {
    team::BST<int, int> tree;
    tree.insert(50, 0);
    tree.insert(40, 0);
    CHECK(*tree.min().key == 40);
    tree.insert(10, 0);
    CHECK(*tree.min().key == 10);
    tree.insert(90, 0);
    CHECK(*tree.max().key == 90);
}

TEST_CASE("the value can be changed through min") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    tree.insert(30, 300);
    *tree.min().value = 123;
    REQUIRE(tree.find(30) != nullptr);
    CHECK(*tree.find(30) == 123);
}

TEST_CASE("walks on an empty tree write nothing") {
    team::BST<int, int> tree;
    team::BST<int, int>::Entry out[5];
    CHECK(tree.ascending(5, out) == 0);
    CHECK(tree.descending(5, out) == 0);
}

// limit 0: nothing is written even when the tree has keys
TEST_CASE("walks with a limit of 0 write nothing") {
    team::BST<int, int> tree;
    tree.insert(50, 0);
    tree.insert(30, 0);
    team::BST<int, int>::Entry out[1];
    CHECK(tree.ascending(0, out) == 0);
    CHECK(tree.descending(0, out) == 0);
}

// one node: both walks return just that node
TEST_CASE("walks on a one-node tree") {
    team::BST<int, int> tree;
    tree.insert(50, 500);
    team::BST<int, int>::Entry out[5];
    REQUIRE(tree.ascending(5, out) == 1);
    CHECK(*out[0].key == 50);
    CHECK(*out[0].value == 500);
    REQUIRE(tree.descending(5, out) == 1);
    CHECK(*out[0].key == 50);
}

// ascending visits every key smallest first
TEST_CASE("ascending returns all keys in increasing order") {
    team::BST<int, int> tree;
    int keys[7] = {50, 30, 70, 20, 40, 60, 80};
    for (int k : keys) tree.insert(k, k * 10);
    team::BST<int, int>::Entry out[7];
    REQUIRE(tree.ascending(7, out) == 7);
    int expected[7] = {20, 30, 40, 50, 60, 70, 80};
    for (int i = 0; i < 7; ++i) {
        CHECK(*out[i].key == expected[i]);
        CHECK(*out[i].value == expected[i] * 10);   // each value stays with its own key
    }
}

// descending visits every key largest first
TEST_CASE("descending returns all keys in decreasing order") {
    team::BST<int, int> tree;
    int keys[7] = {50, 30, 70, 20, 40, 60, 80};
    for (int k : keys) tree.insert(k, k * 10);
    team::BST<int, int>::Entry out[7];
    REQUIRE(tree.descending(7, out) == 7);
    int expected[7] = {80, 70, 60, 50, 40, 30, 20};
    for (int i = 0; i < 7; ++i) {
        CHECK(*out[i].key == expected[i]);
        CHECK(*out[i].value == expected[i] * 10);
    }
}

// a small limit stops early with the best few keys
TEST_CASE("walks stop after limit entries") {
    team::BST<int, int> tree;
    int keys[7] = {50, 30, 70, 20, 40, 60, 80};
    for (int k : keys) tree.insert(k, 0);
    team::BST<int, int>::Entry out[3];

    REQUIRE(tree.ascending(3, out) == 3);
    CHECK(*out[0].key == 20);
    CHECK(*out[1].key == 30);
    CHECK(*out[2].key == 40);

    REQUIRE(tree.descending(3, out) == 3);
    CHECK(*out[0].key == 80);
    CHECK(*out[1].key == 70);
    CHECK(*out[2].key == 60);
}

// a limit bigger than the tree returns only what exists
TEST_CASE("walks with a limit larger than the tree return size() entries") {
    team::BST<int, int> tree;
    tree.insert(50, 0);
    tree.insert(30, 0);
    tree.insert(70, 0);
    team::BST<int, int>::Entry out[10];
    CHECK(tree.ascending(10, out) == 3);
    CHECK(tree.descending(10, out) == 3);
}

// long chain (every key goes right, like W2): order is still right
TEST_CASE("walks work on a chain of increasing keys") {
    team::BST<int, int> tree;
    for (int k = 1; k <= 5; ++k) tree.insert(k, 0);
    team::BST<int, int>::Entry out[5];
    REQUIRE(tree.ascending(5, out) == 5);
    for (int i = 0; i < 5; ++i) CHECK(*out[i].key == i + 1);
    REQUIRE(tree.descending(2, out) == 2);
    CHECK(*out[0].key == 5);
    CHECK(*out[1].key == 4);
}