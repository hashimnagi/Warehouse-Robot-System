#include <iostream>
#include "ChargingStation.h"

ChargingStation::ChargingStation(string stationID, string stationLocationID, int slotCount)
{
    id = stationID;
    locationID = stationLocationID;
    totalSlots = slotCount;
    next = NULL;
}

string ChargingStation::getID()
{
    return id;
}

string ChargingStation::getLocationID()
{
    return locationID;
}

int ChargingStation::getTotalSlots()
{
    return totalSlots;
}

int ChargingStation::getFreeSlots()
{
    int freeCount = 0;

    for (int i = 0; i < totalSlots; i++)
    {
        if (slots[i] == "")
        {
            freeCount++;
        }
    }

    return freeCount;
}

bool ChargingStation::hasFreeSlot()
{
    return getFreeSlots() > 0;
}

bool ChargingStation::isRobotCharging(string robotID)
{
    if (robotID == "")
    {
        return false;
    }

    for (int i = 0; i < totalSlots; i++)
    {
        if (slots[i] == robotID)
        {
            return true;
        }
    }

    return false;
}

bool ChargingStation::startCharging(string robotID)
{
    if (robotID == "" || isRobotCharging(robotID))
    {
        return false;
    }

    for (int i = 0; i < totalSlots; i++)
    {
        if (slots[i] == "")
        {
            slots[i] = robotID;
            return true;
        }
    }

    return false;
}

bool ChargingStation::finishCharging(string robotID)
{
    if (robotID == "")
    {
        return false;
    }

    for (int i = 0; i < totalSlots; i++)
    {
        if (slots[i] == robotID)
        {
            slots[i] = "";
            return true;
        }
    }

    return false;
}

ChargingQueue& ChargingStation::getQueue()
{
    return waitingQueue;
}

ChargingStation* ChargingStation::getNext()
{
    return next;
}

void ChargingStation::display()
{
    cout << "Station " << id << " at " << locationID << endl;
    cout << "    Slots: " << totalSlots << " total, " << getFreeSlots() << " free" << endl;
    cout << "    Charging: ";

    int printed = 0;

    for (int i = 0; i < totalSlots; i++)
    {
        if (slots[i] != "")
        {
            if (printed > 0)
            {
                cout << ", ";
            }

            cout << slots[i];
            printed++;
        }
    }

    if (printed == 0)
    {
        cout << "none";
    }

    cout << endl;
    cout << "    Waiting: ";
    waitingQueue.display();
}