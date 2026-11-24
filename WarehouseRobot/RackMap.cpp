#include <iostream>
#include <fstream>
#include <sstream>
#include "RackMap.h"

RackLocationMap::RackLocationMap()
{
    head = NULL;
}

RackLocationMap::~RackLocationMap()
{
    RackMapEntry* current = head;

    while (current != NULL)
    {
        RackMapEntry* temp = current->next;
        delete current;
        current = temp;
    }
}

string RackLocationMap::getLocation(string rackID)
{
    RackMapEntry* current = head;

    while (current != NULL)
    {
        if (current->rackID == rackID)
        {
            return current->locationID;
        }

        current = current->next;
    }

    return "";
}

bool RackLocationMap::load(string fileName, Graph& graph)
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
        string rackID;
        string locationID;

        lineReader >> rackID >> locationID;

        if (lineReader.fail())
        {
            cout << "Line " << lineNumber << ": missing field, skipped." << endl;
            continue;
        }

        if (getLocation(rackID) != "")
        {
            cout << "Line " << lineNumber << ": duplicate rack ID, skipped." << endl;
            continue;
        }

        Location* location = graph.findLocation(locationID);

        if (location == NULL)
        {
            cout << "Line " << lineNumber << ": location " << locationID << " does not exist, skipped." << endl;
            continue;
        }

        if (location->getType() != RACK)
        {
            cout << "Line " << lineNumber << ": location " << locationID << " is not a RACK location, skipped." << endl;
            continue;
        }

        RackMapEntry* newEntry = new RackMapEntry;
        newEntry->rackID = rackID;
        newEntry->locationID = locationID;
        newEntry->next = head;
        head = newEntry;
        loadedCount++;
    }

    cout << loadedCount << " racks loaded." << endl;
    return true;
}