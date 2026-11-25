#include <iostream>
#include "Graph.h"
#include "ChargingNetwork.h"
using namespace std;

static void showChoice(ChargingNetwork& network, Graph& warehouse, string location, double battery, double rate)
{
    StationChoice choice = network.findBestStation(warehouse, location, battery, rate);

    if (choice.station == NULL)
    {
        cout << location << ": " << choice.reason << endl;
        return;
    }

    cout << location << ": station " << choice.station->getID() << ", distance " << choice.distance;

    if (choice.hasFreeSlot)
    {
        cout << ", free slot" << endl;
    }
    else
    {
        cout << ", no free slot, join the queue" << endl;
    }
}

int main()
{
    cout << boolalpha;
    Graph warehouse;
    ChargingNetwork network;
    warehouse.loadLocations("data\\locations.txt");
    warehouse.loadPathways("data\\pathways.txt");
    network.load("data\\chargers.txt", warehouse);
    ChargingStation* c01 = network.findStation("C01");
    ChargingStation* c03 = network.findStation("C03");

    cout << "--- all stations free ---" << endl;
    showChoice(network, warehouse, "L07", 15, 0.25);

    cout << "--- C01 is full ---" << endl;
    cout << "RB02 request at C01: " << network.requestCharging("RB02", c01) << endl;
    showChoice(network, warehouse, "L07", 15, 0.25);

    cout << "--- all slots full ---" << endl;
    network.requestCharging("RB03", network.findStation("C02"));
    network.requestCharging("RB04", network.findStation("C02"));
    network.requestCharging("RB05", c03);
    showChoice(network, warehouse, "L07", 15, 0.25);

    cout << "--- queue rules ---" << endl;
    cout << "RB01 request at C01: " << network.requestCharging("RB01", c01) << endl;
    cout << "RB01 request again: " << network.requestCharging("RB01", c01) << endl;
    cout << "RB02 request at C03: " << network.requestCharging("RB02", c03) << endl;
    cout << "RB06 request at C01: " << network.requestCharging("RB06", c01) << endl;
    c01->display();

    cout << "--- complete charging ---" << endl;
    string startedRobot;
    cout << "RB02 completes: " << network.completeCharging("RB02", startedRobot) << endl;
    cout << "Next robot started: " << startedRobot << endl;
    c01->display();
    cout << "RB99 completes: " << network.completeCharging("RB99", startedRobot) << endl;
    cout << "RB06 removed from the queue: " << network.removeFromQueue("RB06") << endl;
    cout << "RB06 removed again: " << network.removeFromQueue("RB06") << endl;

    cout << "--- battery and reachability ---" << endl;
    showChoice(network, warehouse, "L07", 0.5, 0.25);
    warehouse.addLocation("L16", RACK, "Isolated_Rack");
    showChoice(network, warehouse, "L16", 50, 0.25);
    showChoice(network, warehouse, "L99", 50, 0.25);
    warehouse.addPathway("L16", "L10", 5);
    warehouse.addPathway("L16", "L14", 5);
    showChoice(network, warehouse, "L16", 50, 0.25);
    return 0;
}