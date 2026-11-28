#include "robot.h"
#include "WarehouseInput.h"
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <cmath>

// Reject invalid attributes even when a derived constructor is used directly.
Robot::Robot(const std::string& id, const std::string& location, double battery,
             double capacity, RobotType robotType)
    : robotID(id), locationID(location), batteryLevel(battery), maxCapacity(capacity),
      type(robotType), status(AVAILABLE) {
    bool validCapacity = std::isfinite(capacity) && capacity > 0 &&
        ((type == PICKER && capacity <= 15) ||
         (type == CARRIER && capacity >= 15 && capacity <= 30) ||
         (type == HEAVY_DUTY && capacity <= 60));
    if (!validCapacity || !validWarehouseID(id, "RB") ||
        !validWarehouseID(location, "L") || !std::isfinite(battery) || battery < 0 || battery > 100)
        throw std::invalid_argument("Invalid robot attributes or type capacity.");
}

// A leg uses the weight carried before reaching its destination.
double Robot::consumption(int distance, double carriedWeight) const {
    if (distance < 0 || !std::isfinite(carriedWeight) || carriedWeight < 0 ||
        carriedWeight > maxCapacity + 1e-9) return -1;
    return distance * batteryRate() * (1.0 + 0.5 * carriedWeight / maxCapacity);
}

const char* Robot::statusName(RobotStatus value) {
    switch (value) {
    case AVAILABLE: return "Available";
    case ASSIGNED: return "Assigned";
    case WAITING_FOR_CHARGING: return "Waiting for Charging";
    case CHARGING_STATUS: return "Charging";
    case UNAVAILABLE: return "Unavailable";
    }
    return "Unknown";
}

const char* Robot::typeName(RobotType value) {
    switch (value) {
    case PICKER: return "Picker";
    case CARRIER: return "Carrier";
    case HEAVY_DUTY: return "Heavy-duty";
    }
    return "Unknown";
}

void Robot::display(std::ostream& output) const {
    std::ios::fmtflags flags = output.flags();
    std::streamsize precision = output.precision();
    output << robotID << " | " << typeName(type) << " | " << locationID
           << " | " << std::fixed << std::setprecision(1) << batteryLevel << "% | "
           << maxCapacity << " kg | " << statusName(status)
           << " | task: " << (assignedTask.empty() ? "none" : assignedTask);
    if (needsCharging()) output << " | needs charging";
    output << '\n';
    output.flags(flags);
    output.precision(precision);
}
