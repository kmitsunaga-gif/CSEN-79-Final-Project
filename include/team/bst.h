#pragma once
#include <cstdlib>  // Provides NULL and size_t
#include <cassert>
#include <iomanip>
#include <vector>
#include <cstddef>

namespace team {
    template <class Key, class Value>
    class BST{
        public:
            //Constuctor 

            void insert(const Key& key, const Value& value);
            std::size_t size() const { return node_count;}
            bool empty() const {return node_count == 0;}
            

        private:
            struct Node {
                Key key;
                Value value;
                Node* left = nullptr;
                Node* right = nullptr;
            };

            Node* root = nullptr;

            std::size_t node_count = 0;
    };
}