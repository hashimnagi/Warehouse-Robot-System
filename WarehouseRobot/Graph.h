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
    Location* findLocation(string id);
    Location* getFirstLocation();
    int getLocationCount();
    void displayAll();
};