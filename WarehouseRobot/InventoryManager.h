#pragma once
#include "AVLTree.h"
#include "Graph.h"

// Ek rack se kitna stock lena hai, uski ek line
struct AllocationLine {
    std::string productID;
    std::string rackID;
    std::string locationID;
    int quantity;
    AllocationLine* next;
};

// RackAllocation ek proposal store karta hai; actual reservation reserveStock() mein hoti hai
class RackAllocation {
    friend class InventoryManager;
private:
    AllocationLine* first;
    AllocationLine* last;
    int requested;
    int allocated;
    bool partialAllowed;

    void append(const std::string& productID, const std::string& rackID,
                const std::string& locationID, int quantity);

public:
    RackAllocation();
    ~RackAllocation();

    // Copy allow nahi hai, sirf move constructor diya hai taaki function return kar sake
    RackAllocation(const RackAllocation&) = delete;
    RackAllocation& operator=(const RackAllocation&) = delete;
    RackAllocation(RackAllocation&& source) noexcept;

    const AllocationLine* lines() const { return first; }
    int requestedQuantity() const { return requested; }
    int allocatedQuantity() const { return allocated; }
    int shortage() const { return requested - allocated; }
    bool reservable() const {
        if (allocated <= 0) {
            return false;
        }
        return (allocated == requested) || partialAllowed;
    }
};

// Module 2: Inventory Management System
class InventoryManager {
private:
    // Rack ID ko Graph ke Location pointer se map karne ka node
    struct RackMapping {
        std::string rackID;
        Location* location;
        RackMapping* next;
    };

    Graph& warehouse;
    AVLTree products;
    RackMapping* mappings;

    RackMapping* findMapping(const std::string& rackID) const;
    RackEntry* lookupEntry(const std::string& productID, const std::string& rackID) const;

public:
    using DistanceLookup = int (*)(const char*, const char*);
    using QuantityChanged = void (*)(const std::string&, const std::string&, int, int);

    explicit InventoryManager(Graph& warehouseGraph);
    ~InventoryManager();

    InventoryManager(const InventoryManager&) = delete;
    InventoryManager& operator=(const InventoryManager&) = delete;

    void setDistanceLookup(DistanceLookup lookup) { distanceLookup = lookup; }
    void setQuantityHistory(QuantityChanged record) { recordChange = record; }

    // File loading functions
    bool loadProducts(const std::string& fileName);
    bool loadRacks(const std::string& fileName);
    bool loadInventory(const std::string& fileName);

    // Product operations
    bool addProduct(const std::string& productID, const std::string& name, double weight, int reorderLevel);
    bool deleteProduct(const std::string& productID);
    Product* searchProduct(const std::string& productID) const;
    double getUnitWeight(const std::string& productID) const;
    long long getTotalStock(const std::string& productID) const;
    long long getAvailableStock(const std::string& productID) const;
    int getReservedQty(const std::string& productID, const std::string& rackID) const;

    // Rack entry operations
    bool addRackEntry(const std::string& productID, const std::string& rackID, int quantity);
    bool removeRackEntry(const std::string& productID, const std::string& rackID);
    bool updateQuantity(const std::string& productID, const std::string& rackID, int newTotal);
    bool revertQuantityUpdate(const std::string& productID, const std::string& rackID, int oldTotal);

    // Display functions
    void displayRacks(const std::string& productID) const;
    void displayLowStock() const;

    // Allocation and reservation functions
    RackAllocation allocateRacks(const std::string& productID, int quantity,
                                 const std::string& referenceLocation, bool allowPartial = false) const;

    // Multi-product orders ke liye saare proposals ek saath pass kar sakte hain
    bool reserveStock(const RackAllocation* allocation, const std::string& orderID);
    struct PlanPart {
        const RackAllocation& allocation;
        const PlanPart* next;
    };
    bool reserveStock(const PlanPart& plan, const std::string& orderID);

    bool releaseReservation(const std::string& orderID, const std::string& productID,
                            const std::string& rackID, int quantity);
    bool deductStock(const std::string& productID, const std::string& rackID,
                     int quantity, const std::string& orderID);
    void releaseOrderReservations(const std::string& orderID);

    void showInventoryMenu();

private:
    DistanceLookup distanceLookup;
    QuantityChanged recordChange;
};
