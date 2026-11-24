#pragma once
#include <string>
#include "Common.h"
#include "Location.h"
using namespace std;

class Route;

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
    bool loadLocations(string fileName);
    bool loadPathways(string fileName);

    Location* findLocation(string id);
    Location* getFirstLocation();
    int getLocationCount();
    void resetSearchData();
    void displayAll();

    bool isConnected(string fromID, string toID);
    Route* getShortestRoute(string fromID, string toID);
    int getDistance(string fromID, string toID);
    bool isPathwayOpen(string fromID, string toID);
};