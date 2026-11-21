#include <iostream>
#include "Graph.h"
using namespace std;

int main()
{
    Graph warehouse;
    warehouse.loadLocations("data\\bad_locations.txt");
    cout << "Count: " << warehouse.getLocationCount() << endl;
    warehouse.displayAll();
    return 0;
}