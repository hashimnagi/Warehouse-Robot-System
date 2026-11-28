#pragma once
#include "PlanGenerator.h"

// Executes a validated trip as one sequential transaction, with no undo record.
class Executor {
private:
    Graph& warehouse;
    InventoryManager& inventory;
    OrderManager& orders;
    RobotList& fleet;
    PlanGenerator& planner;
public:
    Executor(Graph& graph, InventoryManager& stock, OrderManager& orderManager,
             RobotList& robots, PlanGenerator& generator);
    bool recalculateRoute(const std::string& orderID, std::string& reason);
    bool executeTrip(const std::string& orderID, std::string& reason);
};
