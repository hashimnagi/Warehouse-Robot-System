#include "carrier.h"

Carrier::Carrier(const std::string& id, const std::string& location, double battery, double capacity)
    : Robot(id, location, battery, capacity, CARRIER) {}

// Carriers have no additional product restriction beyond physical capacity.
bool Carrier::isEligible(double orderWeight, double heaviestUnit) const {
    return orderWeight > 0 && heaviestUnit > 0 && heaviestUnit <= getMaxCapacity();
}
