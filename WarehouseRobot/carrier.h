#pragma once
#include "robot.h"

class Carrier : public Robot {
public:
    Carrier(const std::string& id, const std::string& location, double battery, double capacity);
    double batteryRate() const override { return 0.25; }
    bool isEligible(double orderWeight, double heaviestUnit) const override;
};
