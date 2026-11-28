#include "PickingPlan.h"
#include "robot.h"
#include <iomanip>
#include <utility>
#include <cmath>
#include <iostream>

PickingStop::PickingStop(const AllocationLine& line, int units, double weight)
    : productID(line.productID), rackID(line.rackID), locationID(line.locationID),
      quantity(units), unitWeight(weight), collected(false), route(nullptr), next(nullptr) {}
PickingStop::~PickingStop() { delete route; }

PickingTrip::PickingTrip()
    : stops(nullptr), lastStop(nullptr), deliveryRoute(nullptr), distance(0), battery(0),
      weight(0), executed(false), next(nullptr) {}
PickingTrip::~PickingTrip() {
    while (stops != nullptr) {
        PickingStop* old = stops;
        stops = stops->next;
        delete old;
    }
    delete deliveryRoute;
}
void PickingTrip::append(PickingStop* stop) {
    if (lastStop != nullptr) lastStop->next = stop;
    else stops = stop;
    lastStop = stop;
}
void deleteTrips(PickingTrip* trips) {
    while (trips != nullptr) {
        PickingTrip* old = trips;
        trips = trips->next;
        delete old;
    }
}
PlanAllocation::PlanAllocation(const std::string& product, int demand, RackAllocation&& proposal)
    : productID(product), remainingDemand(demand), allocation(std::move(proposal)), next(nullptr) {}
PickingPlan::PickingPlan(const std::string& order, const std::string& robot)
    : orderID(order), robotID(robot), allocations(nullptr), trips(nullptr), next(nullptr),
      assigned(false), cancelled(false), partial(false), tripCount(0), totalDistance(0),
      estimatedBattery(0) {}
PickingPlan::~PickingPlan() {
    deleteTrips(trips);
    while (allocations != nullptr) {
        PlanAllocation* old = allocations;
        allocations = allocations->next;
        delete old;
    }
}
PickingTrip* PickingPlan::nextTrip() const {
    for (PickingTrip* trip = trips; trip != nullptr; trip = trip->next)
        if (!trip->executed) return trip;
    return nullptr;
}
void PickingPlan::updateTotals() {
    tripCount = 0;
    totalDistance = 0;
    estimatedBattery = 0;
    for (PickingTrip* trip = trips; trip != nullptr; trip = trip->next) {
        ++tripCount;
        totalDistance += trip->distance;
        estimatedBattery += trip->battery;
    }
}

// Keep completed trips as history and atomically replace only the remaining suffix.
void PickingPlan::replaceRemaining(PickingTrip* replacement) {
    PickingTrip** link = &trips;
    while (*link != nullptr && (*link)->executed) link = &(*link)->next;
    deleteTrips(*link);
    *link = replacement;
    updateTotals();
}

void PickingPlan::display() const {
    std::ios::fmtflags flags = std::cout.flags();
    std::streamsize precision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(1)
              << "\nOrder: " << orderID << " | Robot: " << robotID
              << (partial ? " | partial stock plan" : "")
              << (cancelled ? " | cancelled" : "") << '\n';
    int number = 0;
    for (PickingTrip* trip = trips; trip != nullptr; trip = trip->next) {
        std::cout << "Trip " << ++number << (trip->executed ? " (delivered)" : " (pending)")
                  << " | Start: " << trip->startLocation << '\n';
        for (PickingStop* stop = trip->stops; stop != nullptr; stop = stop->next) {
            std::cout << "  Visit " << stop->rackID << " at " << stop->locationID
                      << ": collect " << stop->productID << " x " << stop->quantity
                      << (stop->collected ? " (collected)" : "") << '\n';
            if (stop->route != nullptr) stop->route->display();
        }
        std::cout << "  Deliver to PACKING " << trip->packingLocation << '\n';
        if (trip->deliveryRoute != nullptr) trip->deliveryRoute->display();
        std::cout << "  Distance: " << trip->distance << " | Weight: " << trip->weight
                  << " kg | Battery: " << trip->battery << "%\n";
    }
    std::cout << "Total: " << tripCount << " trips, " << totalDistance
              << " distance units, " << estimatedBattery << "% battery (+10% reserve).\n";
    std::cout.flags(flags);
    std::cout.precision(precision);
}

// Re-run Dijkstra between the same pickup and delivery stops in a temporary list.
PickingTrip* rebuildRemainingRoutes(const PickingPlan& plan, const Robot& robot,
                                   Graph& graph, std::string& reason) {
    PickingTrip* first = nullptr;
    PickingTrip* tail = nullptr;
    std::string current = robot.getLocationID();
    for (PickingTrip* old = plan.nextTrip(); old != nullptr; old = old->next) {
        PickingTrip* trip = new PickingTrip;
        if (tail != nullptr) tail->next = trip;
        else first = trip;
        tail = trip;
        trip->startLocation = current;
        trip->packingLocation = old->packingLocation;
        for (PickingStop* source = old->stops; source != nullptr; source = source->next) {
            if (source->collected || source->quantity <= 0) {
                reason = "A pending stop is already collected or has an invalid quantity.";
                deleteTrips(first);
                return nullptr;
            }
            AllocationLine line{source->productID, source->rackID, source->locationID,
                                source->quantity, nullptr};
            PickingStop* stop = new PickingStop(line, source->quantity, source->unitWeight);
            trip->append(stop);
            stop->route = graph.getShortestRoute(current, stop->locationID);
            if (stop->route == nullptr) {
                reason = "Rack " + stop->rackID + " is unreachable. Use Cancel Assignment to re-plan.";
                deleteTrips(first);
                return nullptr;
            }
            int distance = stop->route->getTotalDistance();
            trip->distance += distance;
            trip->battery += robot.consumption(distance, trip->weight);
            trip->weight += stop->quantity * stop->unitWeight;
            if (!std::isfinite(trip->weight) || trip->weight > robot.getMaxCapacity() + 1e-9) {
                reason = "Trip exceeds carrying capacity.";
                deleteTrips(first);
                return nullptr;
            }
            current = stop->locationID;
        }
        Location* packing = graph.findLocation(trip->packingLocation);
        if (packing == nullptr || packing->getType() != PACKING) {
            reason = "The delivery location is not a packing station.";
            deleteTrips(first);
            return nullptr;
        }
        trip->deliveryRoute = graph.getShortestRoute(current, trip->packingLocation);
        if (trip->deliveryRoute == nullptr) {
            reason = "Packing station is unreachable. Use Cancel Assignment to re-plan.";
            deleteTrips(first);
            return nullptr;
        }
        int distance = trip->deliveryRoute->getTotalDistance();
        trip->distance += distance;
        trip->battery += robot.consumption(distance, trip->weight);
        current = trip->packingLocation;
    }
    return first;
}
