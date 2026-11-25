#include <iostream>
#include <fstream>
#include <sstream>
#include "ChargingNetwork.h"

static bool isValidStationID(string id)
{
    if (id.length() < 2 || id[0] != 'C')
    {
        return false;
    }

    for (size_t i = 1; i < id.length(); i++)
    {
        if (id[i] < '0' || id[i] > '9')
        {
            return false;
        }
    }

    return true;
}

static bool isSmallerID(string first, string second)
{
    if (first.length() != second.length())
    {
        return first.length() < second.length();
    }

    return first < second;
}

ChargingNetwork::ChargingNetwork()
{
    head = NULL;
    stationCount = 0;
}

ChargingNetwork::~ChargingNetwork()
{
    ChargingStation* current = head;

    while (current != NULL)
    {
        ChargingStation* temp = current->next;
        delete current;
        current = temp;
    }
}

bool ChargingNetwork::load(string fileName, Graph& graph)
{
    ifstream file(fileName);

    if (!file.is_open())
    {
        cout << "Could not open " << fileName << endl;
        return false;
    }

    string line;
    int lineNumber = 0;
    int loadedCount = 0;

    while (getline(file, line))
    {
        lineNumber++;

        if (line == "")
        {
            continue;
        }

        stringstream lineReader(line);
        string stationID;
        string locationID;
        int slotCount;
        string extra;

        lineReader >> stationID >> locationID >> slotCount;

        if (lineReader.fail())
        {
            cout << "Line " << lineNumber << ": missing field or slots is not a number, skipped." << endl;
            continue;
        }

        if (lineReader >> extra)
        {
            cout << "Line " << lineNumber << ": too many fields, skipped." << endl;
            continue;
        }

        if (!isValidStationID(stationID))
        {
            cout << "Line " << lineNumber << ": invalid station ID " << stationID << ", skipped." << endl;
            continue;
        }

        if (findStation(stationID) != NULL)
        {
            cout << "Line " << lineNumber << ": duplicate station ID, skipped." << endl;
            continue;
        }

        Location* location = graph.findLocation(locationID);

        if (location == NULL)
        {
            cout << "Line " << lineNumber << ": location " << locationID << " does not exist, skipped." << endl;
            continue;
        }

        if (location->getType() != CHARGING)
        {
            cout << "Line " << lineNumber << ": location " << locationID << " is not a CHARGING location, skipped." << endl;
            continue;
        }

        if (slotCount < 1 || slotCount > MAX_SLOTS)
        {
            cout << "Line " << lineNumber << ": slots must be from 1 to " << MAX_SLOTS << ", skipped." << endl;
            continue;
        }

        ChargingStation* newStation = new ChargingStation(stationID, locationID, slotCount);

        if (head == NULL)
        {
            head = newStation;
        }
        else
        {
            ChargingStation* last = head;

            while (last->next != NULL)
            {
                last = last->next;
            }

            last->next = newStation;
        }

        stationCount++;
        loadedCount++;
    }

    cout << loadedCount << " stations loaded." << endl;
    return true;
}

ChargingStation* ChargingNetwork::findStation(string stationID)
{
    ChargingStation* current = head;

    while (current != NULL)
    {
        if (current->getID() == stationID)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

ChargingStation* ChargingNetwork::findStationCharging(string robotID)
{
    ChargingStation* current = head;

    while (current != NULL)
    {
        if (current->isRobotCharging(robotID))
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

ChargingStation* ChargingNetwork::findStationQueueing(string robotID)
{
    ChargingStation* current = head;

    while (current != NULL)
    {
        if (current->getQueue().contains(robotID))
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

ChargingStation* ChargingNetwork::getFirstStation()
{
    return head;
}

int ChargingNetwork::getStationCount()
{
    return stationCount;
}

void ChargingNetwork::displayAll()
{
    if (head == NULL)
    {
        cout << "No charging stations are loaded." << endl;
        return;
    }

    ChargingStation* current = head;

    while (current != NULL)
    {
        current->display();
        current = current->next;
    }
}

StationChoice ChargingNetwork::findBestStation(Graph& graph, string fromLocationID, double battery, double batteryRate)
{
    StationChoice choice;
    choice.station = NULL;
    choice.distance = 0;
    choice.hasFreeSlot = false;
    choice.reason = "";

    if (graph.findLocation(fromLocationID) == NULL)
    {
        choice.reason = "Location " + fromLocationID + " does not exist.";
        return choice;
    }

    ChargingStation* bestFree = NULL;
    int bestFreeDistance = 0;
    ChargingStation* bestAny = NULL;
    int bestAnyDistance = 0;
    bool anyReachable = false;
    ChargingStation* current = head;

    while (current != NULL)
    {
        int distance = graph.getDistance(fromLocationID, current->getLocationID());

        if (distance >= 0)
        {
            anyReachable = true;

            if (distance * batteryRate <= battery)
            {
                if (bestAny == NULL || distance < bestAnyDistance
                    || (distance == bestAnyDistance && isSmallerID(current->getID(), bestAny->getID())))
                {
                    bestAny = current;
                    bestAnyDistance = distance;
                }

                if (current->hasFreeSlot())
                {
                    if (bestFree == NULL || distance < bestFreeDistance
                        || (distance == bestFreeDistance && isSmallerID(current->getID(), bestFree->getID())))
                    {
                        bestFree = current;
                        bestFreeDistance = distance;
                    }
                }
            }
        }

        current = current->next;
    }

    if (bestFree != NULL)
    {
        choice.station = bestFree;
        choice.distance = bestFreeDistance;
        choice.hasFreeSlot = true;
    }
    else if (bestAny != NULL)
    {
        choice.station = bestAny;
        choice.distance = bestAnyDistance;
        choice.hasFreeSlot = false;
    }
    else if (!anyReachable)
    {
        choice.reason = "No charging station is reachable from " + fromLocationID + ".";
    }
    else
    {
        choice.reason = "Not enough battery to reach any charging station. Handle the robot manually (set it Unavailable).";
    }

    return choice;
}

int ChargingNetwork::requestCharging(string robotID, ChargingStation* station)
{
    if (robotID == "" || station == NULL)
    {
        return CHARGE_REFUSED;
    }

    if (findStationCharging(robotID) != NULL || findStationQueueing(robotID) != NULL)
    {
        return CHARGE_REFUSED;
    }

    if (station->hasFreeSlot())
    {
        station->startCharging(robotID);
        return CHARGE_STARTED;
    }

    station->getQueue().enqueue(robotID);
    return CHARGE_QUEUED;
}

bool ChargingNetwork::completeCharging(string robotID, string& startedRobotID)
{
    startedRobotID = "";
    ChargingStation* station = findStationCharging(robotID);

    if (station == NULL)
    {
        return false;
    }

    station->finishCharging(robotID);

    if (!station->getQueue().isEmpty())
    {
        startedRobotID = station->getQueue().dequeue();
        station->startCharging(startedRobotID);
    }

    return true;
}

bool ChargingNetwork::removeFromQueue(string robotID)
{
    ChargingStation* station = findStationQueueing(robotID);

    if (station == NULL)
    {
        return false;
    }

    return station->getQueue().remove(robotID);
}