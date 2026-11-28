#pragma once
#include "Common.h"
#include <string>
#include <iosfwd>

class RobotList;
class FleetSupport;

// The fleet owns each polymorphic robot; other modules borrow its pointer.
class Robot {
    friend class RobotList;
    friend class FleetSupport;
private:
    std::string robotID;
    std::string locationID;
    double batteryLevel;
    double maxCapacity;
    RobotType type;
    RobotStatus status;
    std::string assignedTask;
protected:
    Robot(const std::string& id, const std::string& location, double battery,
          double capacity, RobotType robotType);
public:
    virtual ~Robot() = default;
    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;
    const std::string& getRobotID() const { return robotID; }
    const std::string& getLocationID() const { return locationID; }
    double getBatteryLevel() const { return batteryLevel; }
    double getMaxCapacity() const { return maxCapacity; }
    RobotType getType() const { return type; }
    RobotStatus getStatus() const { return status; }
    const std::string& getAssignedTask() const { return assignedTask; }
    bool needsCharging() const { return batteryLevel < 20.0; }
    virtual double batteryRate() const = 0;
    virtual bool isEligible(double orderWeight, double heaviestUnit) const = 0;
    double consumption(int distance, double carriedWeight) const;
    void display(std::ostream& output) const;
    static const char* statusName(RobotStatus status);
    static const char* typeName(RobotType type);
};
