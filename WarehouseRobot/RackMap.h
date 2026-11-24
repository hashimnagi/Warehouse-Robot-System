#pragma once
#include <string>
#include "Graph.h"
using namespace std;

struct RackMapEntry
{
    string rackID;
    string locationID;
    RackMapEntry* next;
};

class RackLocationMap
{
private:
    RackMapEntry* head;

public:
    RackLocationMap();
    ~RackLocationMap();
    bool load(string fileName, Graph& graph);
    string getLocation(string rackID);
};