#pragma once
#include "robot.h"

class HeavyDuty : public Robot {
public:
    HeavyDuty(const std::string& id, const std::string& location, double battery, double capacity);
    double batteryRate() const override { return 0.35; }
    bool isEligible(double orderWeight, double heaviestUnit) const override;
};
