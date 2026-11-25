#pragma once
#include "OrderList.h"
#include "OrderQueue.h"
#include <iostream>

class InventoryManager;

// Module 3: Customer Order Management System
class OrderManager {
private:
    InventoryManager& inventory;
    OrderList orders;
    OrderQueue pendingOrders;

    // Person C ka callback jo assigned robot ko free karta hai
    using ReleaseRobot = bool (*)(const std::string& orderID);
    ReleaseRobot releaseRobot;

    bool releaseAssignment(Order& order);
    bool addRecord(std::istream& fields);

public:
    explicit OrderManager(InventoryManager& inventoryManager);

    void setRobotRelease(ReleaseRobot callback) { releaseRobot = callback; }

    bool loadOrders(const std::string& fileName);
    bool addOrder(const std::string& orderID, const std::string& customerID,
                  const std::string& firstProductID, int quantity);
    bool addItem(const std::string& orderID, const std::string& productID, int quantity);

    Order* searchOrder(const std::string& orderID) const;
    Order* peekFrontOrder() const;
    Order* dequeueOrder();
    int queueSize() const;

    bool requeueAtFront(Order* order);
    bool cancelOrder(const std::string& orderID);
    bool updateOrderStatus(const std::string& orderID, OrderStatus newStatus);
    bool markTripStarted(const std::string& orderID);

    // quantity batata hai ki abhi packing station par kitne naye units deliver hue
    bool updateDeliveredQty(const std::string& orderID, const std::string& productID, int quantity);

    void displayPending() const;
    void displayCompleted() const;
    void showOrderMenu();
};
