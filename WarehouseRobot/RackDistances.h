#pragma once
#include "Graph.h"

// Yeh class Graph se read-only shortest distance nikalne ke liye hai
// Location ke andar koi field change nahi karti
class RackDistances {
private:
    struct DistanceEntry {
        Location* location;
        long long distance;
        bool visited;
        DistanceEntry* next;
    };

    DistanceEntry* entries;
    DistanceEntry* find(Location* location) const;

public:
    RackDistances(Graph& warehouse, const std::string& referenceLocation);
    ~RackDistances();

    RackDistances(const RackDistances&) = delete;
    RackDistances& operator=(const RackDistances&) = delete;

    long long distanceTo(Location* location) const;
};
