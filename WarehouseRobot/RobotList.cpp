#include "RobotList.h"
#include "Graph.h"
#include "UndoStack.h"
#include "WarehouseInput.h"
#include "picker.h"
#include "carrier.h"
#include "heavy-duty.h"
#include <cmath>

RobotList::RobotList(Graph& graph) : warehouse(graph), head(nullptr), tail(nullptr) {}

RobotList::~RobotList() {
    while (head != nullptr) {
        RobotNode* node = head;
        head = head->next;
        delete node->robot;
        delete node;
    }
}

Robot* RobotList::searchRobot(const std::string& id) const {
    for (RobotNode* node = head; node != nullptr; node = node->next)
        if (node->robot->getRobotID() == id) return node->robot;
    return nullptr;
}

// Validate before allocating a robot and linking it into the fleet.
bool RobotList::registerRobot(const std::string& id, RobotType type, const std::string& location,
                             double battery, double capacity) {
    if (!validWarehouseID(id, "RB") || searchRobot(id) != nullptr ||
        !validWarehouseID(location, "L") || warehouse.findLocation(location) == nullptr ||
        !std::isfinite(battery) || battery < 0 || battery > 100 ||
        !std::isfinite(capacity) || capacity <= 0) return false;
    Robot* robot = nullptr;
    if (type == PICKER && capacity <= 15) robot = new Picker(id, location, battery, capacity);
    else if (type == CARRIER && capacity >= 15 && capacity <= 30)
        robot = new Carrier(id, location, battery, capacity);
    else if (type == HEAVY_DUTY && capacity <= 60)
        robot = new HeavyDuty(id, location, battery, capacity);
    else return false;
    RobotNode* node = new RobotNode{robot, nullptr};
    if (tail != nullptr) tail->next = node;
    else head = node;
    tail = node;
    return true;
}

// Rows contain robotID type locationID battery capacity; initial robots are idle.
bool RobotList::loadRobots(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file) { std::cerr << "Could not open " << fileName << '\n'; return false; }
    std::string line;
    int lineNumber = 0;
    bool valid = true;
    while (std::getline(file, line)) {
        ++lineNumber;
        std::istringstream fields(line);
        if (endOfRecord(fields)) continue;
        std::string id, typeText, location;
        double battery = 0, capacity = 0;
        RobotType type = PICKER;
        bool rowOK = static_cast<bool>(fields >> id >> typeText >> location >> battery >> capacity);
        if (typeText == "Picker" || typeText == "PICKER") type = PICKER;
        else if (typeText == "Carrier" || typeText == "CARRIER") type = CARRIER;
        else if (typeText == "Heavy-duty" || typeText == "HEAVY_DUTY") type = HEAVY_DUTY;
        else rowOK = false;
        if (!rowOK || !endOfRecord(fields) || !registerRobot(id, type, location, battery, capacity)) {
            std::cerr << fileName << ", line " << lineNumber << " rejected.\n";
            valid = false;
        }
    }
    return valid && !file.bad();
}

void RobotList::displayAll(bool availableOnly) const {
    bool found = false;
    for (RobotNode* node = head; node != nullptr; node = node->next) {
        if (!availableOnly || node->robot->getStatus() == AVAILABLE) {
            node->robot->display(std::cout);
            found = true;
        }
    }
    if (!found) std::cout << "No matching robots.\n";
}

bool RobotList::updateBattery(const std::string& id, double battery) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || !std::isfinite(battery) || battery < 0 || battery > 100) return false;
    robot->batteryLevel = battery;
    return true;
}

bool RobotList::updateLocation(const std::string& id, const std::string& location) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || !validWarehouseID(location, "L") ||
        warehouse.findLocation(location) == nullptr) return false;
    robot->locationID = location;
    return true;
}

bool RobotList::assignRobot(const std::string& id, const std::string& task) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || task.empty() || robot->status != AVAILABLE ||
        !robot->assignedTask.empty()) return false;
    for (RobotNode* node = head; node != nullptr; node = node->next)
        if (node->robot->assignedTask == task) return false;
    robot->assignedTask = task;
    robot->status = ASSIGNED;
    return true;
}

bool RobotList::releaseRobot(const std::string& id) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || robot->status != ASSIGNED) return false;
    robot->assignedTask.clear();
    robot->status = AVAILABLE; // needsCharging() flags a low battery separately.
    return true;
}

// Use Person A's stack for maintenance changes; restoration never records itself.
bool RobotList::setMaintenance(const std::string& id, bool unavailable, UndoStack* history) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || !robot->assignedTask.empty() ||
        (robot->status != AVAILABLE && robot->status != UNAVAILABLE)) return false;
    RobotStatus target = unavailable ? UNAVAILABLE : AVAILABLE;
    if (target == robot->status) return true;
    if (history != nullptr) history->pushMaintenanceChange(id, robot->status);
    robot->status = target;
    return true;
}

bool RobotList::revertMaintenance(const std::string& id, RobotStatus oldStatus) {
    Robot* robot = searchRobot(id);
    if (robot == nullptr || !robot->assignedTask.empty() ||
        (robot->status != AVAILABLE && robot->status != UNAVAILABLE) ||
        (oldStatus != AVAILABLE && oldStatus != UNAVAILABLE)) return false;
    robot->status = oldStatus;
    return true;
}
