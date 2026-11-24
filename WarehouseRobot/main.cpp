#include <iostream>
#include "Graph.h"
#include "RackMap.h"
using namespace std;

int main()
{
    Graph warehouse;
    RackLocationMap racks;
    warehouse.loadLocations("data\\locations.txt");
    warehouse.loadPathways("data\\pathways.txt");
    racks.load("data\\bad_racks.txt", warehouse);

    cout << "Rack RK04 is at: " << racks.getLocation("RK04") << endl;
    cout << "Rack RK01 is at: " << racks.getLocation("RK01") << endl;
    cout << "Rack RK09 is at: [" << racks.getLocation("RK09") << "]" << endl;
    return 0;
}