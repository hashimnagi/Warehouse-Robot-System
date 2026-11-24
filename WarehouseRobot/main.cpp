#include <iostream>
#include "Graph.h"
#include "ChargingNetwork.h"
using namespace std;

int main()
{
    Graph warehouse;
    ChargingNetwork network;
    warehouse.loadLocations("data\\locations.txt");
    warehouse.loadPathways("data\\pathways.txt");
    network.load("data\\chargers.txt", warehouse);
    network.displayAll();

    ChargingStation* c01 = network.findStation("C01");
    c01->startCharging("RB02");
    c01->getQueue().enqueue("RB01");
    c01->getQueue().enqueue("RB03");
    cout << endl << "RB02 charging, RB01 and RB03 waiting:" << endl;
    c01->display();

    c01->finishCharging("RB02");
    string nextRobot = c01->getQueue().dequeue();
    c01->startCharging(nextRobot);
    cout << endl << "RB02 finished, " << nextRobot << " starts:" << endl;
    c01->display();

    cout << endl << "RB03 removed from the queue: " << boolalpha << c01->getQueue().remove("RB03") << endl;
    cout << "RB03 removed again: " << c01->getQueue().remove("RB03") << endl;
    c01->display();
    return 0;
}