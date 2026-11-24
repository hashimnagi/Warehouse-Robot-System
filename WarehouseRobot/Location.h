#pragma once
#include <climits>
#include <string>
#include "Common.h"
using namespace std;

enum PathStatus
{
    OPEN,
    BLOCKED
};

class Location;

struct Edge
{
    Location* destination;
    int distance;
    PathStatus status;
    Edge* next;
};

class Location
{
private:
    string id;
    LocationType type;
    string name;
    Edge* edgeHead;
    Location* next;

    int searchDistance;
    int searchHops;
    bool searchVisited;
    Location* searchPrevious;

    friend class Graph;

public:
    Location(string locationID, LocationType locationType, string locationName);
    ~Location();

    string getID();
    LocationType getType();
    string getName();
    Location* getNext();
    Edge* getFirstEdge();

    bool addEdge(Location* destination, int distance, PathStatus status);
    bool removeEdge(string destinationID);
    Edge* findEdge(string destinationID);
    int countEdges();
    void resetSearchData();
};