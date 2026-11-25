//AvlMap.h
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <utility>
#include "kit/counters.h"


namespace final_project_map {
    template <class Key, class Value>
    class binary_tree_node
    {
    public:        
        // CONSTRUCTOR
        //     Postcondition: The new node has its data equal to init_data,
        //     and it's child pointers equal to init_left and init_right.   
        binary_tree_node(const Key& key,
                 binary_tree_node* init_left = nullptr,
                 binary_tree_node* init_right = nullptr)
            : data_field(key, Value()),      
            left_field(init_left),
            right_field(init_right),
            height_field(1) { }

        int  height() const { return height_field; }
        void set_height(int h) { height_field = h; }
        
        // MODIFICATION MEMBER FUNCTIONS
        //     Postcondition: The return value is a reference to the data from
        //     this binary_tree_node.
        std::pair<Key, Value>& data( ) { return data_field; }

        const Key& getKey() const { return data_field.first; }
        Value& getValue() { return data_field.second; }
        const Value& getValue() const { return data_field.second; }

        //Postcondition: the function returns a reference to a pointer to either the left or right child
        binary_tree_node*& left( ) { return left_field; }
        binary_tree_node*& right( ) { return right_field; }

        //     Postcondition: The binary_tree_node now contains the specified new link
        //     to a child.
        void set_left(binary_tree_node* new_left) { left_field = new_left; }
        void set_right(binary_tree_node* new_right) { right_field = new_right; }
        
        // CONST MEMBER FUNCTIONS
        //     Postcondition: The return value is a reference to the data from
        //     this binary_tree_node.
        const std::pair<Key, Value>& data( ) const { return data_field; }

        //     Postcondition: The return value is a pointer to the left or right child
        //     (which will be NULL if there is no child).
        const binary_tree_node* left( ) const { return left_field; }
        const binary_tree_node* right( ) const { return right_field; }

        //     Postcondition: The return value is true if the node is a leaf;
        //     otherwise the return value is false.
        bool is_leaf( ) const { return (left_field == nullptr) && (right_field == nullptr); }
        
    private:
        std::pair<Key, Value> data_field; // data field of a node
        binary_tree_node *left_field;  // pointer to the left sub-tree
        binary_tree_node *right_field; // pointer to the right sub-tree
        int height_field;   // a leaf has height 1; a null pointer counts as 0
    };

    template <class Key, class Value, class Compare = std::less<Key>>
    class AvlMap {
        using Node = binary_tree_node<Key, Value>;
        public:
            AvlMap() {
                root = nullptr;
            }
            ~AvlMap() {
                mapClear(root);
            }

            //DOUBLE CHECK THIS
            AvlMap(const AvlMap&) = delete;              // copy constructor: AvlMap b = a;
            AvlMap& operator=(const AvlMap&) = delete;   // assignment: b = a;
            
            // //ADD COPY CONSTRUCTOR AND ASSIGNMENT OPERATOR LATER, IF NEEDED

            // // void insert(const Key& key, order* userOrder);    //userOrder = the order placed/filled/cancelled to add to the history
            // Value& operator [](const Key& key);
            // Value* at(const Key& key);
            // bool find(const Key& key);

            const kit::Counters& stats() const { return stats_; }
            void reset_stats() { stats_.reset(); }

            Value& operator [](const Key& key) {
                return find_or_create(key)->getValue();
            }

            Value* at(const Key& key) {
                 Node* cursor = root;
                while(cursor != nullptr) {
                    KIT_COUNT(stats_, visits);
                    if(less(cursor->getKey(), key)) {  //cursor->getKey() < key
                        cursor = cursor->right();
                    }
                    else if(less(key, cursor->getKey())) {     //cursor->getKey() > key
                        cursor = cursor->left();
                    }
                    else {
                        return &(cursor->getValue());
                    }
                }

                return nullptr;     //key wasn't found in the map
            }

            bool erase(const Key& key) {
                bool erased = false;
                root = eraseNode(root, key, erased);
                return erased;
            }

            bool find(const Key& key) {
                 Node* cursor = root;
                while(cursor != nullptr) {
                    KIT_COUNT(stats_, visits);
                    if(less(cursor->getKey(), key)) {      //cursor->getKey() < key
                        cursor = cursor->right();
                    }
                    else if(less(key, cursor->getKey())) {     //cursor->getKey() > key
                        cursor = cursor->left();
                    }
                    else {
                        return true;
                    }
                }
                return false;
            }

            // First entry in comparator order (best price), or nullptr if empty.
            std::pair<Key, Value>* first() {
                Node* c = root;
                KIT_COUNT(stats_, visits);
                while(c != nullptr && c->left() != nullptr) 
                    c = c->left();
                if(c != nullptr)
                    return &c->data();
                return nullptr;
            }         

            // Visits entries in order; visit(entry) returns false to stop early.
            template <class F>
            void forEachInOrder(F visit) { inOrder(root, visit); }
        private:
            //  Node *root;
            // void mapClear( Node* node);
            //  Node* find_or_create(const std::string& key);
            // int calcBalanceFactor( Node* node);
            // int height( Node* node);
            mutable kit::Counters stats_;
            Node* root;
            Compare cmp;        // the comparator object

            bool less(const Key& a, const Key& b) const {
                KIT_COUNT(stats_, comparisons);
                return cmp(a, b);
            }

            static int height( Node* node) { return node ? node->height() : 0; }

            int balanceFactor( Node* node) {
                if(node == nullptr)
                    return 0;
                return (height(node->left()) - height(node->right()));
            }

            void updateHeight( Node* node) {
                node->set_height(1 + std::max(height(node->left()), height(node->right())));
            }

            // Right rotation: y's left child x becomes the subtree root.
             Node* rightRotate(Node* y) {
                KIT_COUNT_EXTRA(stats_, "rotations");
                Node* x = y->left();
                Node* t = x->right();
                x->set_right(y);
                y->set_left(t);
                updateHeight(y);    // y is now lower, so update it first
                updateHeight(x);
                return x;           // new subtree root
            }

            // Left rotation: x's right child y becomes the subtree root.
            Node* leftRotate(Node* x) {
                KIT_COUNT_EXTRA(stats_, "rotations");
                Node* y = x->right();
                Node* t = y->left();
                y->set_left(x);
                x->set_right(t);
                updateHeight(x);
                updateHeight(y);
                return y;
            }

            // Fix one node whose subtrees may have changed. Returns the new subtree root.
            Node* rebalance(Node* node) {
                updateHeight(node);
                int bal = balanceFactor(node);

                if(bal > 1) {                                   // left-heavy
                    if(balanceFactor(node->left()) < 0)         // left-right case
                        node->set_left(leftRotate(node->left()));
                    return rightRotate(node);                   // left-left case
                }
                if(bal < -1) {                                  // right-heavy
                    if(balanceFactor(node->right()) > 0)        // right-left case
                        node->set_right(rightRotate(node->right()));
                    return leftRotate(node);                    // right-right case
                }
                return node;
            }

            // Recursive insert: rebalances every ancestor as the recursion unwinds.
            // Depth is only O(log n), so recursion is ok here.
            // result receives the node holding key (existing or new).
            Node* insertNode(Node* node, const Key& key, Node*& result) {
                if(node == nullptr) {
                    result = new Node(key);
                    KIT_COUNT(stats_, allocations);
                    return result;
                }
                KIT_COUNT(stats_, visits);
                if(less(key, node->getKey()))
                    node->set_left(insertNode(node->left(), key, result));
                else if(less(node->getKey(), key))
                    node->set_right(insertNode(node->right(), key, result));
                else {
                    result = node;
                    return node;
                }
                return rebalance(node);
            }

            // Detaches the leftmost node of a subtree; returns the new subtree root.
            Node* removeMin(Node* node, Node*& minNode) {
                if(node->left() == nullptr) {
                    minNode = node;
                    return node->right();
                }
                KIT_COUNT(stats_, visits);
                node->set_left(removeMin(node->left(), minNode));
                return rebalance(node);
            }

            Node* eraseNode(Node* node, const Key& key, bool& erased) {
                if(node == nullptr) return nullptr;
                
                KIT_COUNT(stats_, visits);
                if(less(key, node->getKey()))    //key < node->getKey() so go left
                    node->set_left(eraseNode(node->left(), key, erased));
                else if(less(node->getKey(), key))   //key > node->getKey() so go right
                    node->set_right(eraseNode(node->right(), key, erased));
                else {
                    erased = true;
                    Node* l = node->left();
                    Node* r = node->right();
                    if(l == nullptr || r == nullptr) {       // 0 or 1 child
                        delete node;
                        if(l != nullptr)
                            return l;
                        return r;
                    }
                    Node* succ = nullptr;                    // 2 children: relink successor
                    Node* newRight = removeMin(r, succ);
                    succ->set_left(l);
                    succ->set_right(newRight);
                    delete node;
                    return rebalance(succ);
                }
                return rebalance(node);
            }

            Node* find_or_create(const Key& key) {
                // Easy case: find an existing user, needs no rebalancing.
                Node* cursor = root;
                while(cursor != nullptr) {
                    KIT_COUNT(stats_, visits);
                    if(less(key, cursor->getKey()))       cursor = cursor->left();
                    else if(less(cursor->getKey(), key))  cursor = cursor->right();
                    else               return cursor;
                }
                // Miss: insert and rebalance on the way back up.
                Node* result = nullptr;
                root = insertNode(root, key, result);
                return result;
            }

            // template <class Key, class Value, class Compare>
            // void insert(const Key& key, order* userOrder) {
            //     Node* node = find_or_create(key);
            //     if(userOrder != nullptr)
            //         node->getValue().push(userOrder);
            // }

            
            void mapClear( Node* node) {
                if(node != nullptr) {
                    mapClear(node->left());
                    mapClear(node->right());
                    delete node;
                }
            }

            template <class F>
            bool inOrder(Node* node, F& visit) {
                if(node == nullptr) return true;

                KIT_COUNT(stats_, visits);
                if(!inOrder(node->left(), visit)) return false;
                if(!visit(node->data())) return false;
                return inOrder(node->right(), visit);
            }
    };
}
