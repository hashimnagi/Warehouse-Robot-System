#include "Executor.h"
#include <cmath>

Executor::Executor(Graph& graph, InventoryManager& stock, OrderManager& orderManager,
                   RobotList& robots, PlanGenerator& generator)
    : warehouse(graph), inventory(stock), orders(orderManager), fleet(robots), planner(generator) {}

// Explicit route recalculation changes only the plan, and only when every leg is reachable.
bool Executor::recalculateRoute(const std::string& orderID, std::string& reason) {
    PickingPlan* plan = planner.findPlan(orderID);
    Robot* robot = plan == nullptr ? nullptr : fleet.searchRobot(plan->robotID);
    if (plan == nullptr || robot == nullptr || !plan->assigned || plan->nextTrip() == nullptr ||
        robot->getAssignedTask() != orderID || robot->getStatus() != ASSIGNED) {
        reason = "No active trip to recalculate.";
        return false;
    }
    PickingTrip* replacement = rebuildRemainingRoutes(*plan, *robot, warehouse, reason);
    if (replacement == nullptr) return false;
    plan->replaceRemaining(replacement);
    reason = "Routes recalculated between the same stops; check estimates before execution.";
    return true;
}

namespace {
// Check each stored route against the current graph, including distance changes.
bool routeOpen(Route* route, Graph& graph) {
    if (route == nullptr || route->getFirstStop() == nullptr) return false;
    long long distance = 0;
    for (RouteStop* stop = route->getFirstStop(); stop != nullptr; stop = stop->next) {
        Location* location = graph.findLocation(stop->locationID);
        if (location == nullptr) return false;
        if (stop->next != nullptr) {
            Edge* edge = location->findEdge(stop->next->locationID);
            if (edge == nullptr || edge->status != OPEN) return false;
            distance += edge->distance;
        }
    }
    return distance == route->getTotalDistance();
}

int reservedFor(const RackEntry& rack, const std::string& orderID) {
    for (ReservationRecord* record = rack.reservations; record != nullptr; record = record->next)
        if (record->orderID == orderID) return record->quantity;
    return 0;
}
}

// All checks use temporary routes; any rejection leaves stock, orders, robots and plans intact.
bool Executor::executeTrip(const std::string& orderID, std::string& reason) {
    PickingPlan* plan = planner.findPlan(orderID);
    Order* order = orders.searchOrder(orderID);
    Robot* robot = plan == nullptr ? nullptr : fleet.searchRobot(plan->robotID);
    if (plan == nullptr || order == nullptr || robot == nullptr || !plan->assigned ||
        plan->nextTrip() == nullptr || robot->getStatus() != ASSIGNED ||
        robot->getAssignedTask() != orderID ||
        (order->getStatus() != ASSIGNED_ORDER && order->getStatus() != PARTIALLY_FULFILLED)) {
        reason = "No assigned unexecuted trip for this order.";
        return false;
    }
    // Inspect saved pathways before rebuilding routes to account for any graph changes.
    bool changed = false;
    for (PickingTrip* trip = plan->nextTrip(); trip != nullptr; trip = trip->next) {
        for (PickingStop* stop = trip->stops; stop != nullptr; stop = stop->next)
            if (!routeOpen(stop->route, warehouse)) changed = true;
        if (!routeOpen(trip->deliveryRoute, warehouse)) changed = true;
    }
    PickingTrip* candidate = rebuildRemainingRoutes(*plan, *robot, warehouse, reason);
    if (candidate == nullptr) return false;
    double remainingBattery = 0;
    for (PickingTrip* trip = candidate; trip != nullptr; trip = trip->next)
        remainingBattery += trip->battery;
    if (!std::isfinite(remainingBattery) || robot->getBatteryLevel() + 1e-9 < remainingBattery + 10) {
        reason = "Insufficient battery for remaining trips plus 10% reserve. No data changed.";
        deleteTrips(candidate);
        return false;
    }
    for (PickingStop* stop = candidate->stops; stop != nullptr; stop = stop->next) {
        Product* product = inventory.searchProduct(stop->productID);
        RackEntry* rack = product == nullptr ? nullptr : product->findRack(stop->rackID);
        long long needed = 0;
        for (PickingStop* other = candidate->stops; other != nullptr; other = other->next)
            if (other->productID == stop->productID && other->rackID == stop->rackID)
                needed += other->quantity;
        if (rack == nullptr || stop->collected || stop->quantity <= 0 ||
            std::fabs(product->unitWeight - stop->unitWeight) > 1e-9 ||
            rack->totalQuantity < needed || rack->reservedQuantity < needed ||
            reservedFor(*rack, orderID) < needed) {
            reason = "Stock, unit weight or this order's reservation is invalid. No data changed.";
            deleteTrips(candidate);
            return false;
        }
        const OrderItem* item = order->items();
        while (item != nullptr && item->productID != stop->productID) item = item->next;
        long long deliveredNow = 0;
        for (PickingStop* other = candidate->stops; other != nullptr; other = other->next)
            if (other->productID == stop->productID) deliveredNow += other->quantity;
        if (item == nullptr || deliveredNow > item->requiredQuantity - item->deliveredQuantity) {
            reason = "Trip would exceed the order's remaining demand. No data changed.";
            deleteTrips(candidate);
            return false;
        }
    }

    // With single-threaded menus these validated API calls cannot fail midway.
    plan->replaceRemaining(candidate);
    PickingTrip* trip = plan->nextTrip();
    orders.markTripStarted(orderID);
    for (PickingStop* stop = trip->stops; stop != nullptr; stop = stop->next) {
        inventory.deductStock(stop->productID, stop->rackID, stop->quantity, orderID);
        stop->collected = true;
        orders.updateDeliveredQty(orderID, stop->productID, stop->quantity);
    }
    fleet.updateLocation(plan->robotID, trip->packingLocation);
    double battery = robot->getBatteryLevel() - trip->battery;
    fleet.updateBattery(plan->robotID, battery < 0 ? 0 : battery);
    trip->executed = true;
    if (plan->nextTrip() == nullptr) {
        fleet.releaseRobot(plan->robotID);
        // A partial-stock plan leaves the order Partially Fulfilled until explicitly requeued.
        reason = order->fullyDelivered() ? "Trip delivered. Order Completed; robot released."
            : "Trip delivered. Order Partially Fulfilled; robot released. Cancel Assignment to re-plan remaining items.";
    } else reason = "Trip delivered. Order Partially Fulfilled; next trip is ready.";
    if (changed) reason = "Changed pathways rerouted. " + reason;
    return true;
}
