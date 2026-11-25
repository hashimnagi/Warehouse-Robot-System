#pragma once
#include "Common.h"
#include <string>

// Order ke andar ek product ki demand aur delivery tracking ka node
struct OrderItem {
    std::string productID;
    int requiredQuantity;
    int deliveredQuantity;
    OrderItem* next;
};

// Ek customer order ko represent karta hai (linked list of OrderItem ke saath)
class Order {
    friend class OrderList;
    friend class OrderQueue;
    friend class OrderManager;

private:
    Order* nextOrder;     // Master list ke liye pointer
    Order* nextPending;   // Pending FIFO queue ke liye pointer
    bool queued;
    bool tripStarted;
    OrderStatus status;
    OrderItem* firstItem;
    OrderItem* lastItem;

    OrderItem* findItem(const std::string& productID) const;
    bool appendItem(const std::string& productID, int quantity);

public:
    const std::string orderID;
    const std::string customerID;

    Order(const std::string& identifier, const std::string& customer);
    ~Order();

    Order(const Order&) = delete;
    Order& operator=(const Order&) = delete;

    OrderStatus getStatus() const { return status; }
    const OrderItem* items() const { return firstItem; }
    // Planner aage ke pending orders dekh sakta hai bina purane orders hataye
    const Order* nextPendingOrder() const { return nextPending; }

    bool hasDelivery() const;
    bool fullyDelivered() const;
    void display() const;
};
