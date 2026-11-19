//
//  doubly_linked_list.h
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#ifndef doubly_linked_list_h
#define doubly_linked_list_h

#include "doubly_linked_list_node.h"
#include <cstddef>
#include <cassert>


namespace CSEN79_StockBook
{

struct ListMetrics //the counters
{
    size_t comparisons = 0;
    size_t nodesVisited = 0;
    size_t allocations = 0;
    size_t deallocations = 0;
    
    void reset()
    {
        comparisons = 0;
        nodesVisited = 0;
        allocations = 0;
        deallocations = 0;
    }
};

/*
 * Doubly Linked List class template
 * - implements doubly linked list with a head pointer (front) and a tail pointer (back).
 * - FIFO ordering: pushBack() enqueues newest at tail; popFront() dequeues oldest from head.
 * Invariant:
 *   - When empty: head_ptr == nullptr
 *   - When not empty: head node's prev == nullptr, and tail node's next == nullptr; head_ptr points to head node and tail_ptr points to tail node.
 * Value Semantics:
 *   - Assignment operator and Copy constructor are provided by user.
 */

template <typename T>
class doubly_linked_list
{
public:
    typedef size_t size_type;
    
    //constructors, deconstructors
    doubly_linked_list()
    {
        head_ptr = nullptr;
        tail_ptr = nullptr;
    }
    
    doubly_linked_list(const T& init_data)
    {
        doubly_linked_list_node<T> * temp = new doubly_linked_list_node<T>(init_data);
        head_ptr = nullptr;
        tail_ptr = nullptr;
    }
    
    ~doubly_linked_list();
    doubly_linked_list(const doubly_linked_list& s);
    
    //assignment operator
    doubly_linked_list& operator=(const doubly_linked_list& s);
    
    //modification member functions
    doubly_linked_list_node<T>* head() const;
    doubly_linked_list_node<T>* tail() const;
    void pushBack(const T& value);
    void popFront();
    T& front();
    bool removeValue(const T& target);
    void clear();
    bool empty() const; //for caller to check preconditions, throw errors accordingly
    //TODO: size_type size() const;
    
    //const member functions
    const T& front() const;
    const ListMetrics& metrics() const
    {
        return metrics_;
    }
    void resetMetrics() const
    {
        metrics_.reset();
    }
    
private:
    doubly_linked_list_node<T> * head_ptr;
    doubly_linked_list_node<T> * tail_ptr;
    //TODO: size_type size = 0;
    
    mutable ListMetrics metrics_;
};




//implementations
//deconstructor
template <typename T>
doubly_linked_list<T>::~doubly_linked_list()
{
    //Postcondition: Release all remaining nodes to prevent memory leaks.
    clear();
}

template <typename T>
doubly_linked_list<T>::doubly_linked_list(const doubly_linked_list<T>& s)
{
    if(s.empty())
    {
        head_ptr = nullptr;
        tail_ptr = nullptr;
        return;
    }
    
    doubly_linked_list_node<T> * temp = s.head();
    while(temp != nullptr)
    {
        pushBack(temp->get_data());
        temp = temp->next();
    }
}

template <typename T>
doubly_linked_list<T>& doubly_linked_list<T>::operator=(const doubly_linked_list<T>& s)
{
    if(this == &s)
    {
        return *this;
    }
    
    clear();
    
    doubly_linked_list_node<T> * temp = s.head();
    while(temp != nullptr)
    {
        pushBack(temp->get_data());
        temp = temp->next();
    }
    
    return *this;
}


template <typename T>
bool doubly_linked_list<T>::empty() const
{
    return (head_ptr == nullptr);
    //Postcondition: return true if head_ptr == nullptr;
}

/*
template <typename T>
size_t doubly_linked_list<T>::size() const
{
    return size;
}
*/

template <typename T>
void doubly_linked_list<T>::pushBack(const T& value)
{
    //Precondition: value is a valid input
    doubly_linked_list_node<T> * temp = new doubly_linked_list_node<T>(value);
    ++metrics_.allocations;
    
    if(head_ptr == nullptr) //if empty
    {
        head_ptr = temp;
        tail_ptr = temp;
    }
    else
    {
        tail_ptr->set_next(temp);
        temp->set_prev(tail_ptr);
        //nodesVisited++ ?
    }
    
    return;
    //Postcondition: appends a node containing the value at tail (back), updates member variables.
}

template <typename T>
void doubly_linked_list<T>::popFront()
{
    //Precondition: linked-list can not be empty
    assert(head_ptr != nullptr);
    
    if(head_ptr->next() == nullptr) //if it's the last node
    {
        delete head_ptr;
        ++metrics_.deallocations;
        head_ptr = nullptr;
        tail_ptr = nullptr;
    }
    else
    {
        doubly_linked_list_node<T> * temp = head_ptr;
        head_ptr = head_ptr->next();
        delete temp;
        ++metrics_.deallocations;
        head_ptr->set_prev(nullptr);
    }
    
    return;
    //Postcondition: removes the node at head (front), updates member variables
}

template <typename T>
T& doubly_linked_list<T>::front()
{
    //Precondition: linked-list can not be empty
    assert(head_ptr != nullptr);
    
    return head_ptr->get_data();
    //Postcondition: returns head node's data
}

template <typename T>
bool doubly_linked_list<T>::removeValue(const T& target)
{
    // TODO: Search from head_ for `target`.
    // For each visited node, increment metrics_.nodesVisited and metrics_.comparisons.
    // When found:
    //   - If prev != nullptr, prev->next = next; else head_ = next;
    //   - If next != nullptr, next->prev = prev;
    //   - delete curr; increment metrics_.deallocations; decrement size_;
    //   - Verify checkInvariant() and return true.
    (void)target;
    return false;
}

template <typename T>
void doubly_linked_list<T>::clear()
{
    if(head_ptr == nullptr) //if empty already
    {
        return;
    }
    
    while(head_ptr != nullptr)
    {
        doubly_linked_list_node<T> * temp = head_ptr;
        head_ptr = head_ptr->next();
        delete temp;
        ++metrics_.deallocations;
    }
    
    head_ptr = nullptr;
    tail_ptr = nullptr;
    
    return;
    //Postcondition: deallocate all nodes in the linked list and updates member variable.
}

#endif // doubly_linked_list_h

}
