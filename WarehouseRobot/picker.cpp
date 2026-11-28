#include "picker.h"

Picker::Picker(const std::string& id, const std::string& location, double battery, double capacity)
    : Robot(id, location, battery, capacity, PICKER) {}

// Pickers cannot carry an individual product heavier than five kilograms.
bool Picker::isEligible(double orderWeight, double heaviestUnit) const {
    return orderWeight > 0 && heaviestUnit > 0 && heaviestUnit <= 5 &&
           heaviestUnit <= getMaxCapacity();
}
