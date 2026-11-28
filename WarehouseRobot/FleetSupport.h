#pragma once
#include "RobotList.h"
#include "ChargingNetwork.h"
#include "InventoryManager.h"
#include "UndoStack.h"

// An adapter connects Person C's robot objects to Person A's existing support structures.
class FleetSupport {
private:
    RobotList& fleet;
    ChargingNetwork& network;
    Graph& warehouse;
    InventoryManager& inventory;
    UndoStack& history;
    bool arriveAtCharger(Robot& robot, ChargingStation& station);
public:
    FleetSupport(RobotList& robots, ChargingNetwork& charging, Graph& graph,
                 InventoryManager& stock, UndoStack& undo);
    bool requestCharging(const std::string& robotID, std::string& reason);
    bool completeCharging(const std::string& robotID, std::string& reason);
    bool removeFromChargingQueue(const std::string& robotID);
    bool undo(std::string& reason);
    void displayNeedsCharging() const;
};
