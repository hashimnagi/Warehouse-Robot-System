#include <climits>
#include "Location.h"

//Constructor to initialize the attributes of Location
Location::Location(string locationID, LocationType locationType, string locationName) 
{
	id = locationID;
	type = locationType;
	name = locationName;
	edgeHead = NULL;
	next = NULL;
	searchPrevious = NULL;
	resetSearchData();
}

//Resets the data every time it is called
void Location::resetSearchData()
{
	searchDistance = INT_MAX;
	searchHops = 0;
	searchVisited = false;
	searchPrevious = NULL;
}

//remove the memory taken from heap  
Location::~Location()
{
	Edge* current = edgeHead;

	while (current != NULL)
	{
		Edge* temp = current->next;
		delete current;
		current = temp;
	}
}
//gets the ID
string Location::getID()
{
	return id;
}
//gets the type of location
LocationType Location::getType()
{
	return type;
}
//gets the name of the location
string Location::getName()
{
	return name;
}
//return the next location in the graph	list
Location* Location::getNext()
{
	return next;
}
//return the first edge
Edge* Location::getFirstEdge()
{
	return edgeHead;
}
//finds the edge between two vertices by their destination ID
Edge* Location::findEdge(string destinationID)
{
	Edge* current = edgeHead;

	while (current != NULL)
	{
		if (current->destination->id == destinationID)
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}

//Adds the edge between two vertices
bool Location::addEdge(Location* destination, int distance, PathStatus status)
{
	if (destination == NULL)
	{
		return false;
	}

	if (findEdge(destination->id) != NULL)
	{
		return false;
	}

	Edge* newEdge = new Edge;
	newEdge->destination = destination;
	newEdge->distance = distance;
	newEdge->status = status;
	newEdge->next = edgeHead;
	edgeHead = newEdge;
	return true;
}

//removes the edges between two vertices by their ID
bool Location::removeEdge(string destinationID)
{
	Edge* previous = NULL, *current = edgeHead;

	while (current != NULL)
	{
		if (current->destination->id == destinationID)
		{
			if (previous == NULL)
				edgeHead = current->next;
			else
				previous->next = current->next;
			delete current;
			return true;
		}
		previous = current;
		current = current->next;
	}
	return false;
}

//counts the total number of edges
int Location::countEdges()
{
	Edge* current = edgeHead;
	int count = 0;

	while (current != NULL)
	{
		count++;
		current = current->next;
	}
	return count;
}


