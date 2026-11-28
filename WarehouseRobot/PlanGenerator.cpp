#include "PlanGenerator.h"
#include <cmath>
#include <climits>
#include <utility>
#include <stdexcept>

PlanGenerator* PlanGenerator::callbackOwner = nullptr;

// Person B's callback has no context parameter, so one warehouse owns it at a time.
PlanGenerator::PlanGenerator(Graph& graph, InventoryManager& stock,
                             OrderManager& orderManager, RobotList& robots)
    : warehouse(graph), inventory(stock), orders(orderManager), fleet(robots), plans(nullptr) {
    if (callbackOwner != nullptr) throw std::logic_error("Only one active planning context is supported.");
    callbackOwner = this;
    orders.setRobotRelease(releaseCallback);
}
PlanGenerator::~PlanGenerator() {
    // Release outstanding reservations before the fleet and inventory are destroyed.
    for (PickingPlan* plan = plans; plan != nullptr; plan = plan->next) {
        if (plan->assigned && !plan->cancelled) {
            inventory.releaseOrderReservations(plan->orderID);
            Robot* robot = fleet.searchRobot(plan->robotID);
            if (robot != nullptr && robot->getAssignedTask() == plan->orderID)
                fleet.releaseRobot(plan->robotID);
        }
    }
    orders.setRobotRelease(nullptr);
    callbackOwner = nullptr;
    while (plans != nullptr) {
        PickingPlan* old = plans;
        plans = plans->next;
        delete old;
    }
}
bool PlanGenerator::releaseCallback(const std::string& orderID) {
    return callbackOwner != nullptr && callbackOwner->releaseAssignment(orderID);
}
PickingPlan* PlanGenerator::findPlan(const std::string& orderID) const {
    for (PickingPlan* plan = plans; plan != nullptr; plan = plan->next)
        if (plan->orderID == orderID && !plan->cancelled) return plan;
    return nullptr;
}

// Called by both order cancellation and requeue; invalidate the plan as well as the robot.
bool PlanGenerator::releaseAssignment(const std::string& orderID) {
    PickingPlan* plan = findPlan(orderID);
    if (plan == nullptr) return true;
    Robot* robot = fleet.searchRobot(plan->robotID);
    if (robot != nullptr && robot->getAssignedTask() == orderID &&
        !fleet.releaseRobot(plan->robotID)) return false;
    plan->cancelled = true;
    plan->assigned = false;
    return true;
}
bool PlanGenerator::cancelAssignment(const std::string& orderID, std::string& reason) {
    Order* order = orders.searchOrder(orderID);
    if (order == nullptr || findPlan(orderID) == nullptr || !orders.requeueAtFront(order)) {
        reason = "Assignment cannot be cancelled (unknown, completed or already cancelled).";
        return false;
    }
    reason = "Uncollected reservations released; order returned to queue front. Deliveries preserved.";
    return true;
}

namespace {
// Working quantities are separate from inventory and are owned by this temporary list.
struct WorkLine {
    const AllocationLine* line;
    int remaining;
    bool sequenced;
    WorkLine* next;
};
class WorkList {
public:
    WorkLine* head;
    WorkLine* tail;
    WorkList() : head(nullptr), tail(nullptr) {}
    ~WorkList() {
        while (head != nullptr) {
            WorkLine* old = head;
            head = head->next;
            delete old;
        }
    }
    WorkList(const WorkList&) = delete;
    WorkList& operator=(const WorkList&) = delete;
    void append(const AllocationLine* line) {
        WorkLine* node = new WorkLine{line, line->quantity, false, nullptr};
        if (tail != nullptr) tail->next = node;
        else head = node;
        tail = node;
    }
};

// Packing ties use lexical LocationID, matching the project's string identifiers.
bool finishTrip(PickingTrip& trip, Robot& robot, Graph& graph, std::string& current) {
    std::string bestID;
    int bestDistance = INT_MAX;
    for (Location* location = graph.getFirstLocation(); location != nullptr; location = location->getNext()) {
        if (location->getType() != PACKING) continue;
        int distance = graph.getDistance(current, location->getID());
        if (distance >= 0 && (bestID.empty() || distance < bestDistance ||
            (distance == bestDistance && location->getID() < bestID))) {
            bestID = location->getID();
            bestDistance = distance;
        }
    }
    if (bestID.empty()) return false;
    trip.packingLocation = bestID;
    trip.deliveryRoute = graph.getShortestRoute(current, bestID);
    if (trip.deliveryRoute == nullptr) return false;
    trip.distance += bestDistance;
    trip.battery += robot.consumption(bestDistance, trip.weight);
    current = bestID;
    return true;
}
}

// Allocate using Person B's deterministic rule, then order racks and split whole units into trips.
PickingPlan* PlanGenerator::buildForRobot(const Order& order, Robot& robot, bool allowPartial,
                                        std::string& reason) {
    PickingPlan* plan = new PickingPlan(order.orderID, robot.getRobotID());
    WorkList work;
    PlanAllocation** allocationTail = &plan->allocations;
    for (const OrderItem* item = order.items(); item != nullptr; item = item->next) {
        int demand = item->requiredQuantity - item->deliveredQuantity;
        if (demand <= 0) continue;
        RackAllocation proposal = inventory.allocateRacks(item->productID, demand,
                                                         robot.getLocationID(), allowPartial);
        if (proposal.shortage() > 0) {
            plan->partial = true;
            if (!allowPartial) {
                reason = "Insufficient reachable stock for " + item->productID + ".";
                delete plan;
                return nullptr;
            }
        }
        PlanAllocation* allocation = new PlanAllocation(item->productID, demand, std::move(proposal));
        *allocationTail = allocation;
        allocationTail = &allocation->next;
        for (const AllocationLine* line = allocation->allocation.lines(); line != nullptr; line = line->next)
            work.append(line);
    }
    if (work.head == nullptr) {
        reason = "No reachable stock is available to collect.";
        delete plan;
        return nullptr;
    }

    // Determine a rack visit order before splitting it into trips. Every line at a rack
    // is collected together; product ties use lexical ProductID for repeatable results.
    WorkList sequence;
    std::string reference = robot.getLocationID();
    while (true) {
        WorkLine* nearest = nullptr;
        int bestDistance = INT_MAX;
        for (WorkLine* line = work.head; line != nullptr; line = line->next) {
            if (line->sequenced) continue;
            int distance = warehouse.getDistance(reference, line->line->locationID);
            if (distance >= 0 && (nearest == nullptr || distance < bestDistance ||
                (distance == bestDistance && line->line->rackID < nearest->line->rackID))) {
                nearest = line;
                bestDistance = distance;
            }
        }
        if (nearest == nullptr) break;
        const std::string rack = nearest->line->rackID;
        reference = nearest->line->locationID;
        while (true) {
            WorkLine* nextProduct = nullptr;
            for (WorkLine* line = work.head; line != nullptr; line = line->next)
                if (!line->sequenced && line->line->rackID == rack &&
                    (nextProduct == nullptr || line->line->productID < nextProduct->line->productID))
                    nextProduct = line;
            if (nextProduct == nullptr) break;
            sequence.append(nextProduct->line);
            nextProduct->sequenced = true;
        }
    }
    for (WorkLine* line = work.head; line != nullptr; line = line->next) {
        if (!line->sequenced) {
            reason = "A planned rack cannot be reached.";
            delete plan;
            return nullptr;
        }
    }

    std::string current = robot.getLocationID();
    PickingTrip* trip = nullptr;
    PickingTrip** tripTail = &plan->trips;
    for (WorkLine* line = sequence.head; line != nullptr; line = line->next) {
        double unitWeight = inventory.getUnitWeight(line->line->productID);
        if (!std::isfinite(unitWeight) || unitWeight <= 0 || unitWeight > robot.getMaxCapacity()) {
            reason = "Capacity cannot hold a single unit.";
            delete plan;
            return nullptr;
        }
        while (line->remaining > 0) {
            if (trip == nullptr) {
                trip = new PickingTrip;
                trip->startLocation = current;
                *tripTail = trip;
                tripTail = &trip->next;
            }
            double freeUnits = std::floor((robot.getMaxCapacity() - trip->weight + 1e-9) / unitWeight);
            int units = freeUnits >= line->remaining ? line->remaining : static_cast<int>(freeUnits);
            if (units <= 0) {
                if (trip->stops == nullptr || !finishTrip(*trip, robot, warehouse, current)) {
                    reason = "No reachable packing station or insufficient capacity.";
                    delete plan;
                    return nullptr;
                }
                trip = nullptr;
                continue;
            }
            PickingStop* stop = new PickingStop(*line->line, units, unitWeight);
            trip->append(stop);
            stop->route = warehouse.getShortestRoute(current, stop->locationID);
            if (stop->route == nullptr) {
                reason = "A pickup route is unreachable.";
                delete plan;
                return nullptr;
            }
            int distance = stop->route->getTotalDistance();
            trip->distance += distance;
            trip->battery += robot.consumption(distance, trip->weight);
            trip->weight += units * unitWeight;
            current = stop->locationID;
            line->remaining -= units;
        }
    }
    if (trip == nullptr || !finishTrip(*trip, robot, warehouse, current)) {
        reason = "No reachable packing station.";
        delete plan;
        return nullptr;
    }
    plan->updateTotals();
    if (!std::isfinite(plan->estimatedBattery) || robot.getBatteryLevel() + 1e-9 < plan->estimatedBattery + 10) {
        reason = "Insufficient battery for all trips plus 10% reserve; charge the robot.";
        delete plan;
        return nullptr;
    }
    return plan;
}

// Evaluate every available robot, then compare trips, distance and RobotID in that order.
PickingPlan* PlanGenerator::previewOrder(const std::string& orderID, bool allowPartial, std::string& reason) {
    Order* order = orders.searchOrder(orderID);
    if (order == nullptr || order->getStatus() != PENDING) {
        reason = "Order is not Pending.";
        return nullptr;
    }
    double totalWeight = 0, heaviest = 0;
    for (const OrderItem* item = order->items(); item != nullptr; item = item->next) {
        int demand = item->requiredQuantity - item->deliveredQuantity;
        if (demand <= 0) continue;
        double weight = inventory.getUnitWeight(item->productID);
        if (!std::isfinite(weight) || weight <= 0) {
            reason = "Missing product or invalid unit weight: " + item->productID;
            return nullptr;
        }
        long long available = inventory.getAvailableStock(item->productID);
        if (!allowPartial && available < demand) {
            reason = "Stock shortage for " + item->productID + ": need " + std::to_string(demand) +
                     ", available " + std::to_string(available) + ".";
            return nullptr;
        }
        totalWeight += demand * weight;
        if (weight > heaviest) heaviest = weight;
    }
    if (!std::isfinite(totalWeight) || totalWeight <= 0) {
        reason = "Order has no valid undelivered demand.";
        return nullptr;
    }
    PickingPlan* best = nullptr;
    bool available = false, capacity = false, eligible = false;
    std::string failures;
    for (const RobotNode* node = fleet.first(); node != nullptr; node = node->next) {
        Robot& robot = *node->robot;
        if (robot.getStatus() != AVAILABLE || !robot.getAssignedTask().empty()) continue;
        available = true;
        if (heaviest > robot.getMaxCapacity()) continue;
        capacity = true;
        if (!robot.isEligible(totalWeight, heaviest)) continue;
        eligible = true;
        std::string failure;
        PickingPlan* candidate = buildForRobot(*order, robot, allowPartial, failure);
        if (candidate == nullptr) {
            failures += robot.getRobotID() + ": " + failure + " ";
            continue;
        }
        if (best == nullptr || candidate->tripCount < best->tripCount ||
            (candidate->tripCount == best->tripCount && candidate->totalDistance < best->totalDistance) ||
            (candidate->tripCount == best->tripCount && candidate->totalDistance == best->totalDistance &&
             candidate->robotID < best->robotID)) {
            delete best;
            best = candidate;
        } else delete candidate;
    }
    if (best != nullptr) { reason.clear(); return best; }
    if (!available) reason = "No available robot.";
    else if (!capacity) reason = "No available robot can carry the heaviest single unit.";
    else if (!eligible) reason = "No available robot passes its type eligibility rule.";
    else reason = failures;
    return nullptr;
}

// Scan the queue without removing skipped orders, preserving their exact positions.
PickingPlan* PlanGenerator::previewNext(bool allowPartial, std::string& reason) {
    std::string skipped;
    for (const Order* order = orders.peekFrontOrder(); order != nullptr; order = order->nextPendingOrder()) {
        std::string failure;
        PickingPlan* plan = previewOrder(order->orderID, allowPartial, failure);
        if (plan != nullptr) { reason = skipped; return plan; }
        skipped += "Skipped " + order->orderID + ": " + failure + "\n";
    }
    reason = skipped.empty() ? "No pending orders." : skipped;
    return nullptr;
}

// Recheck previews and reserve all products through Person B's atomic PlanPart API.
bool PlanGenerator::confirm(PickingPlan* plan, std::string& reason) {
    if (plan == nullptr || plan->assigned || plan->cancelled || findPlan(plan->orderID) != nullptr) {
        reason = "Invalid or already assigned plan.";
        return false;
    }
    Order* order = orders.searchOrder(plan->orderID);
    Robot* robot = fleet.searchRobot(plan->robotID);
    if (order == nullptr || order->getStatus() != PENDING || robot == nullptr ||
        robot->getStatus() != AVAILABLE || !robot->getAssignedTask().empty() || plan->trips == nullptr ||
        robot->getLocationID() != plan->trips->startLocation) {
        reason = "Order or robot changed since preview; generate a new plan.";
        return false;
    }
    for (const RobotNode* node = fleet.first(); node != nullptr; node = node->next) {
        if (node->robot->getAssignedTask() == plan->orderID) {
            reason = "Another robot already holds this task; no stock was reserved.";
            return false;
        }
    }
    double weight = 0, heaviest = 0;
    for (const OrderItem* item = order->items(); item != nullptr; item = item->next) {
        int demand = item->requiredQuantity - item->deliveredQuantity;
        if (demand <= 0) continue;
        PlanAllocation* allocation = plan->allocations;
        while (allocation != nullptr && allocation->productID != item->productID) allocation = allocation->next;
        if (allocation == nullptr ||
            (allocation != nullptr && allocation->remainingDemand != demand)) {
            reason = "Order demand changed since preview.";
            return false;
        }
        double unit = inventory.getUnitWeight(item->productID);
        if (!std::isfinite(unit) || unit <= 0) { reason = "Product no longer exists."; return false; }
        weight += unit * demand;
        if (unit > heaviest) heaviest = unit;
    }
    if (!robot->isEligible(weight, heaviest)) { reason = "Robot is no longer eligible."; return false; }
    for (PickingTrip* trip = plan->trips; trip != nullptr; trip = trip->next) {
        for (PickingStop* stop = trip->stops; stop != nullptr; stop = stop->next) {
            if (std::fabs(inventory.getUnitWeight(stop->productID) - stop->unitWeight) > 1e-9) {
                reason = "Product weight changed since preview; generate a new plan.";
                return false;
            }
        }
    }
    PickingTrip* refreshed = rebuildRemainingRoutes(*plan, *robot, warehouse, reason);
    if (refreshed == nullptr) return false;
    double battery = 0;
    long long distance = 0;
    for (PickingTrip* trip = refreshed; trip != nullptr; trip = trip->next) {
        battery += trip->battery;
        distance += trip->distance;
    }
    deleteTrips(refreshed);
    if (distance != plan->totalDistance || std::fabs(battery - plan->estimatedBattery) > 1e-9 ||
        robot->getBatteryLevel() + 1e-9 < battery + 10) {
        reason = "Routes or battery changed; generate a new plan before confirming.";
        return false;
    }
    InventoryManager::PlanPart* parts = nullptr;
    for (PlanAllocation* allocation = plan->allocations; allocation != nullptr; allocation = allocation->next)
        if (allocation->allocation.allocatedQuantity() > 0)
            parts = new InventoryManager::PlanPart{allocation->allocation, parts};
    bool reserved = parts != nullptr && inventory.reserveStock(*parts, plan->orderID);
    while (parts != nullptr) {
        const InventoryManager::PlanPart* old = parts;
        parts = const_cast<InventoryManager::PlanPart*>(parts->next);
        delete old;
    }
    if (!reserved) { reason = "Stock changed; no inventory was reserved."; return false; }
    // All following transitions are guaranteed by the checks above in this sequential program.
    orders.updateOrderStatus(plan->orderID, RESERVED);
    fleet.assignRobot(plan->robotID, plan->orderID);
    orders.updateOrderStatus(plan->orderID, ASSIGNED_ORDER);
    plan->assigned = true;
    plan->next = plans;
    plans = plan;
    reason = "Plan reserved and assigned. It can be executed now or later.";
    return true;
}
