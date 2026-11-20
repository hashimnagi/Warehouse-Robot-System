#include <iostream>
#include "Location.h"
using namespace std;

int main()
{
    Location* first = new Location("L01", PACKING, "Packing_North");
    Location* second = new Location("L02", RACK, "Rack_Zone_A1");

    cout << first->getID() << " " << first->getName() << endl;
    cout << second->getID() << " " << second->getName() << endl;
    cout << "Add L01 to L02: " << first->addEdge(second, 3, OPEN) << endl;
    cout << "Edges in L01: " << first->countEdges() << endl;

    delete first;
    delete second;
    return 0;
}