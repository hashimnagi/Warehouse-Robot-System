#include <iostream>
#include "Graph.h"

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