//
//  doubly_linked_list.cpp
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#include "doubly_linked_list.h"

namespace CSEN79_StockBook
{

class Order;

// Explicit instantiation for Order* so object files have concrete symbols if linked standalone.
template class doubly_linked_list<Order*>;

}
