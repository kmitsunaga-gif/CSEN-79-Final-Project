#pragma once
#include <cstddef>

namespace team {
    template <class Key, class Value>
    class BST{
        public:
            BST() = default; 

            BST(const BST&) = delete;

            ~BST(){
                tree_clear(root);
            }

            struct Entry{
                const Key* key = nullptr;
                Value* value = nullptr;
            };

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

            Value* find(const Key& key){
                Node* current = root;
                while(current != nullptr){
                    if(key == current -> key){ //if key is equal to the current key then we found our value
                        return &current -> value;
                    } else if (key < current ->key){ //smaller so it reviews left child
                        current = current -> left;
                    } else {
                        current = current -> right; //larger so it reviews right child
                    }
                }
                return nullptr;
            }
            
            Entry min(){
                if(root == nullptr){
                    return {nullptr, nullptr};
                }

                Node* current = root; //set current equal to the root
                while(current -> left != nullptr){ //min will be the leftmost node so we keep going until next is nullptr
                    current = current -> left;
                }
                return {&current->key, &current->value};
            }

            Entry max(){
                if(root == nullptr){
                    return {nullptr, nullptr};
                }

                Node* current = root;
                while(current -> right != nullptr){
                    current = current -> right;
                }
                return {&current -> key, &current -> value};
            }

            std::size_t ascending(std::size_t limit, Entry* out){
                std::size_t count = 0; //local counter 
                collect_ascending(root, limit, out, count); //walks from the top and fills out and updates count
                return count; //returns the number of entries the caller got
            }

            std:: size_t descending(std::size_t limit, Entry* out){
                std::size_t count = 0; //local counter
                collect_descending(root, limit, out, count); //
                return count;
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

            void tree_clear(Node* node)
            {
                Node* child;
                if (node != nullptr)
                {
                    child = node->left;
                    tree_clear(child);
                    
                    child = node->right; //deletes right subtree
                    tree_clear(child);
                    delete node; //deletes the node
                }
            }

            void collect_ascending(Node* node, std::size_t limit, Entry* out, std::size_t& count){
                if(node == nullptr || count == limit){
                    return;
                }

                collect_ascending(node -> left, limit, out, count);
                
                if(count == limit){
                    return;
                }

                out[count] = {&node -> key, &node -> value};
                ++count;

                collect_ascending(node -> right, limit, out, count);
            }

            void collect_descending(Node* node, std::size_t limit, Entry* out, std::size_t& count){
                if(node == nullptr || count == limit){
                    return;
                }

                collect_descending(node -> right, limit, out, count);
                
                if(count == limit){
                    return;
                }

                out[count] = {&node -> key, &node -> value};
                ++count;

                collect_descending(node -> left, limit, out, count);
            }

            Node* root = nullptr;

            std::size_t node_count = 0;
    };
}