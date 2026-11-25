#include "OrderList.h"
#include <iostream>

using namespace std;

OrderList::OrderList(){
    first = nullptr;
    last = nullptr;
}

// Destructor: saare Order objects delete karo kyunki master list inki owner hai
OrderList::~OrderList(){
    Order* curr = first;
    while (curr != nullptr) {
        Order* temp = curr;
        curr = curr->nextOrder;
        delete temp;
    }
    first = nullptr;
    last = nullptr;
}

// List ke end mein naya order add karo
void OrderList::append(Order* order){
    if (last != nullptr) {
        last->nextOrder = order;
    } else {
        first = order;
    }
    last = order;
}

// Order ID se linear search karo
Order* OrderList::find(const string& orderID) const{
    Order* curr = first;
    while (curr != nullptr) {
        if (curr->orderID == orderID) {
            return curr;
        }
        curr = curr->nextOrder;
    }
    return nullptr;
}

// Sirf un orders ko print karo jinka status match karta hai
void OrderList::displayStatus(OrderStatus status) const{
    bool found = false;
    Order* curr = first;
    while (curr != nullptr) {
        if (curr->getStatus() == status) {
            curr->display();
            found = true;
        }
        curr = curr->nextOrder;
    }

    if (!found) {
        cout << "No matching orders.\n";
    }
}
