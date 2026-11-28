#pragma once
#include "robot.h"
class Graph;
class UndoStack;

// The list owns both its nodes and their Robot pointers.
struct RobotNode {
    Robot* robot;
    RobotNode* next;
};

class RobotList {
private:
    Graph& warehouse;
    RobotNode* head;
    RobotNode* tail;
public:
    explicit RobotList(Graph& graph);
    ~RobotList();
    RobotList(const RobotList&) = delete;
    RobotList& operator=(const RobotList&) = delete;
    const RobotNode* first() const { return head; }
    Robot* searchRobot(const std::string& robotID) const;
    bool registerRobot(const std::string& id, RobotType type, const std::string& location,
                       double battery, double capacity);
    bool loadRobots(const std::string& fileName);
    void displayAll(bool availableOnly = false) const;
    bool updateBattery(const std::string& id, double battery);
    bool updateLocation(const std::string& id, const std::string& location);
    bool assignRobot(const std::string& id, const std::string& task);
    bool releaseRobot(const std::string& id);
    bool setMaintenance(const std::string& id, bool unavailable, UndoStack* history = nullptr);
    bool revertMaintenance(const std::string& id, RobotStatus oldStatus);
};
