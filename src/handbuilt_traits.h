//
//  handbuilt_traits.h
//  CSEN79_StockBook_Project
//
//  The hand-built version of the book's structures. Kept separate from stock_book.h
//  so the STL version builds on its own. Include this from main.cpp once every
//  header below exists.
//

#ifndef handbuilt_traits_h
#define handbuilt_traits_h

#include "stock_book.h"
#include "doubly_linked_list_node.h"
#include "B_tree.h"
#include "doubly_linked_list.h"
#include "singly_linked_list.h"
#include "map.h"

namespace CSEN79_StockBook
{

    struct HandBuiltTraits
    {
        using BuyBTree = B_tree<double, doubly_linked_list<Order*>*, 4, std::greater<double>>;
        using SellBTree = B_tree<double, doubly_linked_list<Order*>*, 4, std::less<double>>;
        using IdOrderMap = map<Order::id_type, Order*>; //TODO: Map
        using OrderList = doubly_linked_list<Order*>;
        using forward_list = singly_linked_list<std::pair<Order*, struct historyTag>>; //TODO: singly_linked_list
        using UserHistoryMap = map<std::string, TailForwardList<forward_list>*>;
    };

}

#endif /* handbuilt_traits_h */