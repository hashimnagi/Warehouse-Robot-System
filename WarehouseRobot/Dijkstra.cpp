#include <iostream>
#include <climits>
#include "Dijkstra.h"

Route* Dijkstra::findShortestRoute(Graph& graph, string fromID, string toID)
{
    Location* start = graph.findLocation(fromID);
    Location* end = graph.findLocation(toID);

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

    graph.resetSearchData();
    start->searchDistance = 0;

    while (true)
    {
        Location* best = NULL;
        Location* scan = graph.getFirstLocation();

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

    return route;
}