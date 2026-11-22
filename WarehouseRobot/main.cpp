#include <iostream>
#include "Graph.h"
#include "Dijkstra.h"
using namespace std;

static void showRoute(Graph& graph, string fromID, string toID)
{
    Route* route = Dijkstra::findShortestRoute(graph, fromID, toID);

    if (route == NULL)
    {
        cout << "No route from " << fromID << " to " << toID << "." << endl;
        return;
    }

    route->display();
    delete route;
}

int main()
{
    Graph warehouse;
    warehouse.loadLocations("data\\locations.txt");
    warehouse.loadPathways("data\\pathways.txt");
    warehouse.addLocation("L16", RACK, "Isolated_Rack");

    showRoute(warehouse, "L01", "L08");
    warehouse.blockPathway("L02", "L06");
    showRoute(warehouse, "L01", "L08");
    showRoute(warehouse, "L01", "L10");
    showRoute(warehouse, "L01", "L16");
    showRoute(warehouse, "L01", "L99");
    showRoute(warehouse, "L01", "L01");
    warehouse.reopenPathway("L02", "L06");
    showRoute(warehouse, "L08", "L01");
    return 0;
}