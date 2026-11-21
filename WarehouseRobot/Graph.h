#pragma once
#include <string>
#include "Common.h"
#include "Location.h"
using namespace std;

class Graph
{
private:
    Location* head;
    int locationCount;

public:
    Graph();
    ~Graph();

    bool addLocation(string id, LocationType type, string name);
    bool addPathway(string fromID, string toID, int distance);
    bool removePathway(string fromID, string toID);
    bool blockPathway(string fromID, string toID);
    bool reopenPathway(string fromID, string toID);
    Location* findLocation(string id);
    Location* getFirstLocation();
    int getLocationCount();
    void displayAll();
};