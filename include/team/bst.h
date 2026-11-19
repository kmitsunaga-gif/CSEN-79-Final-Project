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
            bool insert(const Key& key, const Value& value){
                //empty tree check, where the new node is the root
                if (root == nullptr){
                    root = new Node{key, value};
                    ++node_count; //one more key in the tree
                    return true ;
                }

                Node* current = root; //node being compared
                Node* parent = nullptr; //node that trails one behind

                while(current != nullptr){
                    parent = current; //remembers our place
                    if(key < current -> key){ //smaller key
                        current = current -> left;
                    } else if( key > current -> key){ //larger key
                        current = current -> right;
                    } else { //equal key
                        return false;
                    }
                }
                
                if(key < parent -> key){ //attaches as a left child
                    parent -> left = new Node{key, value};
                } else { //attaches as a right child
                    parent -> right = new Node{key, value};
                }

                ++node_count;
                return true;
            }

            std::size_t size() const { return node_count;} //number of keys
            bool empty() const {return node_count == 0;} //true when there are no keys
            

        private:
            struct Node {
                Key key; //price of the node, used to sort the tree
                Value value; //data stored within the key
                Node* left = nullptr;
                Node* right = nullptr;
            };

            Node* root = nullptr;

            std::size_t node_count = 0;
    };
}