#include "heavy-duty.h"

HeavyDuty::HeavyDuty(const std::string& id, const std::string& location, double battery, double capacity)
    : Robot(id, location, battery, capacity, HEAVY_DUTY) {}

// Heavy-duty robots are reserved for orders above 15 kg or a unit above 10 kg.
bool HeavyDuty::isEligible(double orderWeight, double heaviestUnit) const {
    return heaviestUnit > 0 && heaviestUnit <= getMaxCapacity() &&
           (orderWeight > 15 || heaviestUnit > 10);
}
