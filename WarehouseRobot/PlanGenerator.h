#pragma once
#include "PickingPlan.h"
#include "RobotList.h"
#include "OrderManager.h"

class PlanGenerator {
private:
    Graph& warehouse;
    InventoryManager& inventory;
    OrderManager& orders;
    RobotList& fleet;
    PickingPlan* plans;
    static PlanGenerator* callbackOwner;
    static bool releaseCallback(const std::string& orderID);
    bool releaseAssignment(const std::string& orderID);
    PickingPlan* buildForRobot(const Order& order, Robot& robot, bool allowPartial,
                              std::string& reason);
public:
    PlanGenerator(Graph& graph, InventoryManager& stock, OrderManager& orderManager, RobotList& robots);
    ~PlanGenerator();
    PlanGenerator(const PlanGenerator&) = delete;
    PlanGenerator& operator=(const PlanGenerator&) = delete;
    // Caller owns previews until confirm() succeeds; then this manager owns them.
    PickingPlan* previewNext(bool allowPartial, std::string& reason);
    PickingPlan* previewOrder(const std::string& orderID, bool allowPartial, std::string& reason);
    bool confirm(PickingPlan* plan, std::string& reason);
    PickingPlan* findPlan(const std::string& orderID) const;
    bool cancelAssignment(const std::string& orderID, std::string& reason);
};
