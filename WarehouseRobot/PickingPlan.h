#pragma once
#include "InventoryManager.h"
#include "Route.h"

class Robot;

// Each stop owns the route travelled before collecting its whole-unit quantity.
struct PickingStop {
    std::string productID;
    std::string rackID;
    std::string locationID;
    int quantity;
    double unitWeight;
    bool collected;
    Route* route;
    PickingStop* next;
    PickingStop(const AllocationLine& line, int units, double weight);
    ~PickingStop();
    PickingStop(const PickingStop&) = delete;
    PickingStop& operator=(const PickingStop&) = delete;
};

struct PickingTrip {
    std::string startLocation;
    std::string packingLocation;
    PickingStop* stops;
    PickingStop* lastStop;
    Route* deliveryRoute;
    long long distance;
    double battery;
    double weight;
    bool executed;
    PickingTrip* next;
    PickingTrip();
    ~PickingTrip();
    PickingTrip(const PickingTrip&) = delete;
    PickingTrip& operator=(const PickingTrip&) = delete;
    void append(PickingStop* stop);
};

// Preserve Person B's allocation objects for one atomic multi-product reservation.
struct PlanAllocation {
    std::string productID;
    int remainingDemand;
    RackAllocation allocation;
    PlanAllocation* next;
    PlanAllocation(const std::string& product, int demand, RackAllocation&& proposal);
};

class PickingPlan {
public:
    std::string orderID;
    std::string robotID;
    PlanAllocation* allocations;
    PickingTrip* trips;
    PickingPlan* next;
    bool assigned;
    bool cancelled;
    bool partial;
    int tripCount;
    long long totalDistance;
    double estimatedBattery;

    PickingPlan(const std::string& order, const std::string& robot);
    ~PickingPlan();
    PickingPlan(const PickingPlan&) = delete;
    PickingPlan& operator=(const PickingPlan&) = delete;
    PickingTrip* nextTrip() const;
    void updateTotals();
    void display() const;
    void replaceRemaining(PickingTrip* replacement);
};

// The caller owns the returned list. Failure frees temporary routes and changes nothing.
PickingTrip* rebuildRemainingRoutes(const PickingPlan& plan, const Robot& robot,
                                   Graph& graph, std::string& reason);
void deleteTrips(PickingTrip* trips);
