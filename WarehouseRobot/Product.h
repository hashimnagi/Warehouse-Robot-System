#pragma once
#include "RackEntry.h"

// Ek product ki complete details aur uske racks ki linked list
struct Product {
    std::string productID;
    std::string name;
    double unitWeight;
    int reorderLevel;
    RackEntry* racks;

    Product(const std::string& identifier, const std::string& productName, double weight, int threshold);
    ~Product();

    // Copy disallow hai kyunki racks dynamically allocated hain
    Product(const Product&) = delete;
    Product& operator=(const Product&) = delete;

    RackEntry* findRack(const std::string& rackID) const;
    long long totalStock() const;
    long long availableStock() const;
    bool hasReservations() const;
};
