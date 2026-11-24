#include <iostream>
#include <climits>
#include "Graph.h"
#include "Route.h"

Route* Graph::getShortestRoute(string fromID, string toID)
{
    Location* start = findLocation(fromID);
    Location* end = findLocation(toID);

    if (start == NULL)
    {
        cout << "Location " << fromID << " does not exist." << endl;
        return NULL;
    }

    if (end == NULL)
    {
        cout << "Location " << toID << " does not exist." << endl;
        return NULL;
    }

    resetSearchData();
    start->searchDistance = 0;

    while (true)
    {
        Location* best = NULL;
        Location* scan = head;

        while (scan != NULL)
        {
            if (!scan->searchVisited && scan->searchDistance != INT_MAX)
            {
                if (best == NULL || scan->searchDistance < best->searchDistance)
                {
                    best = scan;
                }
            }

            scan = scan->getNext();
        }

        if (best == NULL)
        {
            break;
        }

        best->searchVisited = true;

        if (best == end)
        {
            break;
        }

        Edge* edge = best->getFirstEdge();

        while (edge != NULL)
        {
            Location* neighbor = edge->destination;

            if (edge->status == OPEN && !neighbor->searchVisited)
            {
                int newDistance = best->searchDistance + edge->distance;
                int newHops = best->searchHops + 1;

                if (newDistance < neighbor->searchDistance)
                {
                    neighbor->searchDistance = newDistance;
                    neighbor->searchHops = newHops;
                    neighbor->searchPrevious = best;
                }
                else if (newDistance == neighbor->searchDistance)
                {
                    if (newHops < neighbor->searchHops)
                    {
                        neighbor->searchHops = newHops;
                        neighbor->searchPrevious = best;
                    }
                    else if (newHops == neighbor->searchHops
                        && neighbor->searchPrevious != NULL
                        && best->getID() < neighbor->searchPrevious->getID())
                    {
                        neighbor->searchPrevious = best;
                    }
                }
            }

            edge = edge->next;
        }
    }

    if (end->searchDistance == INT_MAX)
    {
        return NULL;
    }

    Route* route = new Route();
    route->setTotalDistance(end->searchDistance);
    Location* step = end;

    while (step != NULL)
    {
        route->addStopAtFront(step->getID());
        step = step->searchPrevious;
    }

    return route; //returns the head route
}