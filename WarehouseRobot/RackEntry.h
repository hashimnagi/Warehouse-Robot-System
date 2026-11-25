#pragma once
#include <string>

// Ek order ne is rack par kitna stock reserve kiya hai, uska node
struct ReservationRecord {
    std::string orderID;
    int quantity;
    ReservationRecord* next;
};

// Har product ke paas apne racks ki linked list hoti hai
// Aur har RackEntry ke paas us rack ke reservations ki linked list hoti hai
struct RackEntry {
    std::string rackID;
    int totalQuantity;
    int reservedQuantity;
    ReservationRecord* reservations;
    RackEntry* next;

    RackEntry(const std::string& identifier, int quantity);
    ~RackEntry();

    // Copy allow nahi kar rahe kyunki linked list pointers hain
    RackEntry(const RackEntry&) = delete;
    RackEntry& operator=(const RackEntry&) = delete;

    int availableQuantity() const;
    bool reserve(const std::string& orderID, int quantity);
    bool release(const std::string& orderID, int quantity, bool collected = false);
    void releaseAll(const std::string& orderID);
};
