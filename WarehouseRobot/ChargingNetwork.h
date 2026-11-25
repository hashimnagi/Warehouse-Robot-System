#pragma once
#include <string>
#include "Graph.h"
#include "ChargingStation.h"
using namespace std;

const int CHARGE_REFUSED = 0;
const int CHARGE_STARTED = 1;
const int CHARGE_QUEUED = 2;

struct StationChoice
{
    ChargingStation* station;
    int distance;
    bool hasFreeSlot;
    string reason;
};

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

    StationChoice findBestStation(Graph& graph, string fromLocationID, double battery, double batteryRate);
    int requestCharging(string robotID, ChargingStation* station);
    bool completeCharging(string robotID, string& startedRobotID);
    bool removeFromQueue(string robotID);
};