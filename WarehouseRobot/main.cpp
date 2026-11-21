#include <iostream>
#include "Graph.h"
using namespace std;

int main()
{
    cout << boolalpha;
    Graph warehouse;

    warehouse.addLocation("L01", PACKING, "Packing_North");
    warehouse.addLocation("L02", RACK, "Rack_Zone_A1");
    warehouse.addLocation("L03", RACK, "Rack_Zone_A2");

    cout << "Add L01-L02: " << warehouse.addPathway("L01", "L02", 3) << endl;
    cout << "Duplicate: " << warehouse.addPathway("L01", "L02", 3) << endl;
    cout << "Reverse duplicate: " << warehouse.addPathway("L02", "L01", 5) << endl;
    cout << "Missing location: " << warehouse.addPathway("L01", "L09", 4) << endl;
    cout << "Self loop: " << warehouse.addPathway("L01", "L01", 2) << endl;
    cout << "Distance 0: " << warehouse.addPathway("L01", "L03", 0) << endl;
    cout << "Negative distance: " << warehouse.addPathway("L01", "L03", -5) << endl;
    cout << "Add L02-L03: " << warehouse.addPathway("L02", "L03", 4) << endl;
    warehouse.displayAll();

    cout << "Block L01-L02: " << warehouse.blockPathway("L01", "L02") << endl;
    cout << "Block again: " << warehouse.blockPathway("L01", "L02") << endl;
    cout << "Block missing: " << warehouse.blockPathway("L01", "L09") << endl;
    warehouse.displayAll();

    cout << "Reopen L01-L02: " << warehouse.reopenPathway("L01", "L02") << endl;
    cout << "Reopen again: " << warehouse.reopenPathway("L01", "L02") << endl;
    cout << "Reopen missing: " << warehouse.reopenPathway("L01", "L09") << endl;
    warehouse.displayAll();

    cout << "Remove L01-L02: " << warehouse.removePathway("L01", "L02") << endl;
    cout << "Remove again: " << warehouse.removePathway("L01", "L02") << endl;
    cout << "Remove missing: " << warehouse.removePathway("L01", "L09") << endl;
    cout << "Block after remove: " << warehouse.blockPathway("L01", "L02") << endl;
    warehouse.displayAll();

    return 0;
}