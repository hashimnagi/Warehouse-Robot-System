#pragma once
#include "Order.h"

// Pending orders ke liye FIFO queue (Order ka nextPending pointer use hota hai)
class OrderQueue {
private:
    Order* front;
    Order* rear;
    int count;

public:
    OrderQueue();

    OrderQueue(const OrderQueue&) = delete;
    OrderQueue& operator=(const OrderQueue&) = delete;

    bool enqueue(Order* order);
    bool prepend(Order* order);
    bool remove(Order* order);
    Order* dequeue();
    Order* peek() const { return front; }
    int size() const { return count; }
    void display() const;
};
