#pragma once
#include "Order.h"

// Master linked list jo saare orders (pending, completed, cancelled) store karti hai
class OrderList {
private:
    Order* first;
    Order* last;

public:
    OrderList();
    ~OrderList();

    OrderList(const OrderList&) = delete;
    OrderList& operator=(const OrderList&) = delete;

    void append(Order* order);
    Order* find(const std::string& orderID) const;
    void displayStatus(OrderStatus status) const;
};
