#include <iostream>
#include "Graph.h"
using namespace std;

int main()
{
    cout << boolalpha;
    Graph warehouse;

    cout << "Add L01: " << warehouse.addLocation("L01", PACKING, "Packing_North") << endl;
    cout << "Add L02: " << warehouse.addLocation("L02", RACK, "Rack_Zone_A1") << endl;
    cout << "Add L03: " << warehouse.addLocation("L03", RACK, "Rack_Zone_A2") << endl;
    cout << "Add L02 again: " << warehouse.addLocation("L02", RACK, "Rack_Zone_A1") << endl;
    cout << "Add empty ID: " << warehouse.addLocation("", RACK, "Nothing") << endl;
    cout << "Count: " << warehouse.getLocationCount() << endl;
    cout << "Find L02: " << (warehouse.findLocation("L02") != NULL) << endl;
    cout << "Find L09: " << (warehouse.findLocation("L09") != NULL) << endl;

    Location* first = warehouse.findLocation("L01");
    Location* second = warehouse.findLocation("L02");
    first->addEdge(second, 3, OPEN);
    second->addEdge(first, 3, OPEN);

    warehouse.displayAll();
    return 0;
}