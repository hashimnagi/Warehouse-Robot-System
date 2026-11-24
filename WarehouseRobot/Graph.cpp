#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Route.h"

static bool textToStatus(string text, PathStatus& status)
{
    if (text == "OPEN")
    {
        status = OPEN;
        return true;
    }

    if (text == "BLOCKED")
    {
        status = BLOCKED;
        return true;
    }

    return false;
}

static bool textToType(string text, LocationType& type)
{
    if (text == "RACK")
    {
        type = RACK;
        return true;
    }

    if (text == "PACKING")
    {
        type = PACKING;
        return true;
    }

    if (text == "CHARGING")
    {
        type = CHARGING;
        return true;
    }

    return false;
}

//Converts the status enum to string
static string statusToText(PathStatus status)
{
    switch (status)
    {
    case OPEN:
        return "OPEN";
    case BLOCKED:
        return "BLOCKED";
    }
    return "UNKNOWN";
}


static string typeToText(LocationType type)
{
    switch (type)
    {
    case RACK:
        return "RACK";
    case PACKING:
        return "PACKING";
    case CHARGING:
        return "CHARGING";
    }
    return "UNKNOWN";
}

Graph::Graph()
{
    head = NULL;
    locationCount = 0;
}

Graph::~Graph()
{
    Location* current = head;

    while (current != NULL)
    {
        Location* temp = current->next;
        delete current;
        current = temp;
    }
}

//Adds location in the graph
bool Graph::addLocation(string id, LocationType type, string name)
{
    if (id == "")
    {
        return false;
    }

    if (findLocation(id) != NULL)
    {
        return false;
    }

        Location* newLocation = new Location(id, type, name);
        newLocation->next = head;
        head = newLocation;
        locationCount++;    
        return true;
}

//Load loactions from the file
bool Graph::loadLocations(string fileName)
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
        string id;
        string typeText;
        string name;
        string extra;

        lineReader >> id >> typeText >> name;

        if (lineReader.fail())
        {
            cout << "Line " << lineNumber << ": missing field, skipped." << endl;
            continue;
        }

        if (lineReader >> extra)
        {
            cout << "Line " << lineNumber << ": too many fields, skipped." << endl;
            continue;
        }

        LocationType type;

        if (!textToType(typeText, type))
        {
            cout << "Line " << lineNumber << ": invalid type " << typeText << ", skipped." << endl;
            continue;
        }

        if (!addLocation(id, type, name))
        {
            cout << "Line " << lineNumber << ": duplicate or empty ID, skipped." << endl;
            continue;
        }

        loadedCount++;
    }

    cout << loadedCount << " locations loaded." << endl;
    return true;
}

//Load pathways from the file
bool Graph::loadPathways(string fileName)
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
        string fromID;
        string toID;
        int distance;
        string statusText;
        string extra;

        lineReader >> fromID >> toID >> distance >> statusText;

        if (lineReader.fail())
        {
            cout << "Line " << lineNumber << ": missing field or distance is not a number, skipped." << endl;
            continue;
        }

        if (lineReader >> extra)
        {
            cout << "Line " << lineNumber << ": too many fields, skipped." << endl;
            continue;
        }

        PathStatus status;

        if (!textToStatus(statusText, status))
        {
            cout << "Line " << lineNumber << ": invalid status " << statusText << ", skipped." << endl;
            continue;
        }

        if (!addPathway(fromID, toID, distance))
        {
            cout << "Line " << lineNumber << ": pathway rejected (missing location, same location, distance not above 0, or duplicate), skipped." << endl;
            continue;
        }

        if (status == BLOCKED)
        {
            blockPathway(fromID, toID);
        }

        loadedCount++;
    }

    cout << loadedCount << " pathways loaded." << endl;
    return true;
}

//Adds a path between the vertices
bool Graph::addPathway(string fromID, string toID, int distance)
{
    Location* fromLocation = findLocation(fromID);
    Location* toLocation = findLocation(toID);

    if (fromLocation == NULL || toLocation == NULL)
    {
        return false;
    }

    if (fromID == toID)
    {
        return false;
    }

    if (distance <= 0)
    {
        return false;
    }

    if (fromLocation->findEdge(toID) != NULL)
    {
        return false;
    }
        
    if (toLocation->findEdge(fromID) != NULL)
    {
        return false;
    }
    if (!fromLocation->addEdge(toLocation, distance, OPEN))
    {
        return false;
    }

    if (!toLocation->addEdge(fromLocation, distance, OPEN))
    {
        fromLocation->removeEdge(toID);
        return false;
    }

    return true;
}

//removes the path between vertices
bool Graph::removePathway(string fromID, string toID)
{
    Location* fromLocation = findLocation(fromID);
    Location* toLocation = findLocation(toID);

    if (fromLocation == NULL || toLocation == NULL)
    {
        return false;
    }

    if (fromLocation->findEdge(toID) == NULL)
    {
        return false;
    }

    fromLocation->removeEdge(toID);
    toLocation->removeEdge(fromID);

    return true;
}
//blocks the path 
bool Graph::blockPathway(string fromID, string toID)
{
    Location* fromLocation = findLocation(fromID);
    Location* toLocation = findLocation(toID);

    if (fromLocation == NULL || toLocation == NULL)
    {
        return false;
    }

    Edge* forwardEdge = fromLocation->findEdge(toID);
    Edge* backwardEdge = toLocation->findEdge(fromID);

    if (forwardEdge == NULL || backwardEdge == NULL)
    {
        return false;
    }

    if (forwardEdge->status == BLOCKED && backwardEdge->status == BLOCKED)
    {
        return false;
    }

    forwardEdge->status = BLOCKED;
    backwardEdge->status = BLOCKED;

    return true;
}

//gets distance between 2 vertices by their ID's
int Graph::getDistance(string fromID, string toID)
{
    Route* route = getShortestRoute(fromID, toID);

    if (route == NULL)
    {
        return -1;
    }

    int distance = route->getTotalDistance();
    delete route;
    return distance;
}

//checks the path status between two vertices
bool Graph::isPathwayOpen(string fromID, string toID)
{
    Location* fromLocation = findLocation(fromID);

    if (fromLocation == NULL)
    {
        return false;
    }

    Edge* edge = fromLocation->findEdge(toID);

    if (edge == NULL)
    {
        return false;
    }

    return edge->status == OPEN;
}

//Changes both edges of the pathway back to open
bool Graph::reopenPathway(string fromID, string toID)
{
    Location* fromLocation = findLocation(fromID);
    Location* toLocation = findLocation(toID);

    if (fromLocation == NULL || toLocation == NULL)
    {
        return false;
    }

    Edge* forwardEdge = fromLocation->findEdge(toID);
    Edge* backwardEdge = toLocation->findEdge(fromID);

    if (forwardEdge == NULL || backwardEdge == NULL)
    {
        return false;
    }

    if (forwardEdge->status == OPEN && backwardEdge->status == OPEN)
    {
        return false;
    }

    forwardEdge->status = OPEN;
    backwardEdge->status = OPEN;

    return true;
}


//Resets the data of a vertix
void Graph::resetSearchData()
{
    Location* current = head;

    while (current != NULL)
    {
        current->resetSearchData();
        current = current->next;
    }
}


// searches the list for a location with this ID 
Location* Graph::findLocation(string id)
{
    Location* current = head;

    while (current != NULL)
    {
        if (current->getID() == id)
        {
            return current;
        }
        current = current->getNext();
    }
    return NULL;
}
//gets the first location i.e head
Location* Graph::getFirstLocation()
{
    return head;
}

//return the count of the location
int Graph::getLocationCount()
{
    return locationCount;
}

//displays the attributes of the graph
void Graph::displayAll()
{
    if (head == NULL)
    {
        cout << "The graph is empty." << endl;
        return;
    }

    Location* current = head;

    while (current != NULL)
    {
        cout << current->getID() << " [" << typeToText(current->getType()) << "] " << current->getName() << endl;

        Edge* edge = current->getFirstEdge();

        while (edge != NULL)
        {
            cout << "    -> " << edge->destination->getID()
                << "  distance " << edge->distance
                << "  " << statusToText(edge->status) << endl;
                edge = edge->next;
        }

        current = current->getNext();
    }
}