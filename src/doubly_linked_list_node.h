//
//  doubly_linked_list_node.h
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#ifndef doubly_linked_list_node_h
#define doubly_linked_list_node_h

namespace CSEN79_StockBook
{

//doubly_linked_list_node template clsss
template <typename T> //T is data type of data stored in a doubly_linked_list_node
class doubly_linked_list_node
{
public:
    
    //constructors
    doubly_linked_list_node(const T& init_data = T(),
                            doubly_linked_list_node* init_prev = nullptr,
                            doubly_linked_list_node* init_next = nullptr)
    : data(init_data), prev_link(init_prev), next_link(init_next) //member initializer list
    {
    }
    
    doubly_linked_list_node(const doubly_linked_list_node& s);
    
    doubly_linked_list_node& operator=(const doubly_linked_list_node& s);
    
    //modification member functions
    T& get_data();
    doubly_linked_list_node*& prev();
    doubly_linked_list_node*& next();
    void set_data(const T& d);
    void set_prev(doubly_linked_list_node* p);
    void set_next(doubly_linked_list_node* n);
    
    //const member functions
    const T& get_data() const;
    doubly_linked_list_node* const& prev() const;
    doubly_linked_list_node* const& next() const;
    
private:
    T data;
    doubly_linked_list_node * prev_link;
    doubly_linked_list_node * next_link;
};



//implementations

//copy constructor
template <typename T>
doubly_linked_list_node<T>::doubly_linked_list_node(const doubly_linked_list_node& s)
{
    data = s.data;
    prev_link = s.prev_link;
    next_link = s.next_link;
}

template <typename T>
doubly_linked_list_node<T>& doubly_linked_list_node<T>::operator=(const doubly_linked_list_node& s)
{
    data = s.data;
    prev_link = s.prev_link;
    next_link = s.next_link;
}

//modification member functions
template <typename T>
T& doubly_linked_list_node<T>::get_data()
{
    return data;
}

template <typename T>
doubly_linked_list_node<T>*& doubly_linked_list_node<T>::prev()
{
    return prev_link;
}

template <typename T>
doubly_linked_list_node<T>*& doubly_linked_list_node<T>::next()
{
    return next_link;
}

template <typename T>
void doubly_linked_list_node<T>::set_data(const T& d)
{
    data = d;
}

template <typename T>
void doubly_linked_list_node<T>::set_prev(doubly_linked_list_node* p)
{
    prev_link = p;
}

template <typename T>
void doubly_linked_list_node<T>::set_next(doubly_linked_list_node* n)
{
    next_link = n;
}

//const member functions
template <typename T>
const T& doubly_linked_list_node<T>::get_data() const
{
    return data;
}

template <typename T>
doubly_linked_list_node<T>* const& doubly_linked_list_node<T>::prev() const
{
    return prev_link;
}

template <typename T>
doubly_linked_list_node<T>* const& doubly_linked_list_node<T>::next() const
{
    return next_link;
}
}
#endif /* doubly_linked_list_node_h */
