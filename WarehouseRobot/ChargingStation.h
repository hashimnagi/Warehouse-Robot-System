#pragma once
#include <string>
#include "ChargingQueue.h"
using namespace std;

const int MAX_SLOTS = 10;

class ChargingStation
{
private:
    string id;
    string locationID;
    int totalSlots;
    string slots[MAX_SLOTS];
    ChargingQueue waitingQueue;
    ChargingStation* next;

    friend class ChargingNetwork;

public:
    ChargingStation(string stationID, string stationLocationID, int slotCount);

    string getID();
    string getLocationID();
    int getTotalSlots();
    int getFreeSlots();
    bool hasFreeSlot();
    bool isRobotCharging(string robotID);
    bool startCharging(string robotID);
    bool finishCharging(string robotID);
    ChargingQueue& getQueue();
    ChargingStation* getNext();
    void display();
};