#include "FleetSupport.h"
#include <iostream>

FleetSupport::FleetSupport(RobotList& robots, ChargingNetwork& charging, Graph& graph,
                           InventoryManager& stock, UndoStack& undo)
    : fleet(robots), network(charging), warehouse(graph), inventory(stock), history(undo) {}

bool FleetSupport::arriveAtCharger(Robot& robot, ChargingStation& station) {
    int distance = warehouse.getDistance(robot.locationID, station.getLocationID());
    double consumption = robot.consumption(distance, 0);
    if (distance < 0 || consumption < 0 || robot.batteryLevel < consumption) return false;
    robot.batteryLevel -= consumption;
    robot.locationID = station.getLocationID();
    robot.status = CHARGING_STATUS;
    return true;
}

// The network chooses slots and FIFO queues; this adapter keeps actual robot records in sync.
bool FleetSupport::requestCharging(const std::string& robotID, std::string& reason) {
    Robot* robot = fleet.searchRobot(robotID);
    if (robot == nullptr || !robot->assignedTask.empty() || robot->status != AVAILABLE) {
        reason = "Only an available robot without a task may request charging.";
        return false;
    }
    StationChoice choice = network.findBestStation(warehouse, robot->locationID,
                                                  robot->batteryLevel, robot->batteryRate());
    if (choice.station == nullptr) { reason = choice.reason; return false; }
    int result = network.requestCharging(robotID, choice.station);
    if (result == CHARGE_REFUSED) { reason = "Charging request refused."; return false; }
    if (result == CHARGE_STARTED) {
        arriveAtCharger(*robot, *choice.station);
        reason = "Charging started at " + choice.station->getID() + ".";
    } else {
        // Waiting robots remain at their current location until their charging slot opens.
        robot->status = WAITING_FOR_CHARGING;
        reason = "Joined FIFO queue at " + choice.station->getID() + ".";
    }
    return true;
}

// Check the queued robot's travel before letting the network automatically start it.
bool FleetSupport::completeCharging(const std::string& robotID, std::string& reason) {
    Robot* robot = fleet.searchRobot(robotID);
    ChargingStation* station = network.findStationCharging(robotID);
    if (robot == nullptr || station == nullptr || robot->status != CHARGING_STATUS) {
        reason = "Robot is not charging.";
        return false;
    }
    std::string nextID = station->getQueue().getFront();
    Robot* next = nextID.empty() ? nullptr : fleet.searchRobot(nextID);
    if (!nextID.empty()) {
        int distance = next == nullptr ? -1 : warehouse.getDistance(next->locationID, station->getLocationID());
        if (next == nullptr || next->status != WAITING_FOR_CHARGING || !next->assignedTask.empty() ||
            distance < 0 || next->consumption(distance, 0) > next->batteryLevel) {
            reason = "Queue front cannot reach the station now. Remove it from the queue first.";
            return false;
        }
    }
    std::string started;
    if (!network.completeCharging(robotID, started)) return false;
    robot->batteryLevel = 100;
    robot->status = AVAILABLE;
    if (!started.empty()) arriveAtCharger(*next, *station);
    reason = "Charge completed; robot is Available at 100%.";
    if (!started.empty()) reason += " FIFO robot " + started + " started charging.";
    return true;
}

bool FleetSupport::removeFromChargingQueue(const std::string& robotID) {
    Robot* robot = fleet.searchRobot(robotID);
    if (robot == nullptr || robot->status != WAITING_FOR_CHARGING ||
        !network.removeFromQueue(robotID)) return false;
    robot->status = AVAILABLE;
    return true;
}

void FleetSupport::displayNeedsCharging() const {
    bool found = false;
    for (const RobotNode* node = fleet.first(); node != nullptr; node = node->next)
        if (node->robot->needsCharging()) { node->robot->display(std::cout); found = true; }
    if (!found) std::cout << "No robot has battery below 20%.\n";
}

// Use the existing stack and inventory restore API, including Person C's maintenance restore.
bool FleetSupport::undo(std::string& reason) {
    UndoRecord* record = history.peek();
    if (record == nullptr) { reason = "Nothing to undo."; return false; }
    bool restored = false;
    if (record->type == UNDO_MAINTENANCE) {
        restored = fleet.revertMaintenance(record->idOne, static_cast<RobotStatus>(record->oldValue));
    } else if (record->type == UNDO_QUANTITY) {
        restored = inventory.revertQuantityUpdate(record->idOne, record->idTwo, record->oldValue);
    } else if (record->type == UNDO_PATHWAY) {
        Location* location = warehouse.findLocation(record->idOne);
        Edge* edge = location == nullptr ? nullptr : location->findEdge(record->idTwo);
        if (edge == nullptr) restored = true;
        else if (edge->status == record->oldValue) restored = true;
        else if (record->oldValue == OPEN) restored = warehouse.reopenPathway(record->idOne, record->idTwo);
        else restored = warehouse.blockPathway(record->idOne, record->idTwo);
    }
    if (!restored) { reason = "Undo rejected by current stock or robot state; entry retained."; return false; }
    history.pop();
    reason = "Operation undone.";
    return true;
}
