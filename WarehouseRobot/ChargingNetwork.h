#pragma once
#include <string>
#include "Graph.h"
#include "ChargingStation.h"
using namespace std;

class ChargingNetwork
{
private:
    ChargingStation* head;
    int stationCount;

public:
    ChargingNetwork();
    ~ChargingNetwork();

    bool load(string fileName, Graph& graph);
    ChargingStation* findStation(string stationID);
    ChargingStation* findStationCharging(string robotID);
    ChargingStation* findStationQueueing(string robotID);
    ChargingStation* getFirstStation();
    int getStationCount();
    void displayAll();
};