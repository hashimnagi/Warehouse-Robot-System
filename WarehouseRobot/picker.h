#pragma once
#include "robot.h"

class Picker : public Robot {
public:
    Picker(const std::string& id, const std::string& location, double battery, double capacity);
    double batteryRate() const override { return 0.20; }
    bool isEligible(double orderWeight, double heaviestUnit) const override;
};
