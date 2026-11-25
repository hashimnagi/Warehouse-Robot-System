// PERSON_B_DEMO_MAIN define karne par yeh standalone demo program run hota hai
#ifdef PERSON_B_DEMO_MAIN
#include "InventoryManager.h"
#include "OrderManager.h"
#include "WarehouseInput.h"
#include <iostream>

using namespace std;

// Person A ki pathways.txt file read karke Graph mein paths add karo
static bool loadDemoPathways(Graph& warehouse, const string& fileName)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Could not open " << fileName << "\n";
        return false;
    }

    string line;
    int lineNumber = 0;
    bool allValid = true;

    while (getline(file, line)) {
        lineNumber++;
        istringstream fields(line);

        if (endOfRecord(fields)) {
            continue;
        }

        string origin, destination, status;
        int distance = 0;
        bool rowOk = true;

        if (!(fields >> origin >> destination >> distance >> status) || !endOfRecord(fields)) {
            rowOk = false;
        } else if (status != "OPEN" && status != "BLOCKED") {
            rowOk = false;
        } else if (!warehouse.addPathway(origin, destination, distance)) {
            rowOk = false;
        } else if (status == "BLOCKED") {
            // Agar pathway BLOCKED hai toh blockPathway() call karo
            if (!warehouse.blockPathway(origin, destination)) {
                rowOk = false;
            }
        }

        if (!rowOk) {
            cerr << fileName << ", line " << lineNumber << " rejected.\n";
            allValid = false;
        }
    }

    return allValid && !file.bad();
}

int main(int argc, char* argv[])
{
    // Command line arguments se folder paths lo, warna default "data" use karo
    string layoutFolder = "data";
    string stockFolder = "data";
    if (argc > 1) {
        layoutFolder = argv[1];
    }
    if (argc > 2) {
        stockFolder = argv[2];
    }

    Graph warehouse;
    InventoryManager inventory(warehouse);
    OrderManager orders(inventory);

    // Saari data files step-by-step load karo
    if (!warehouse.loadLocations(layoutFolder + "/locations.txt")) return 1;
    if (!loadDemoPathways(warehouse, layoutFolder + "/pathways.txt")) return 1;
    if (!inventory.loadRacks(stockFolder + "/racks.txt")) return 1;
    if (!inventory.loadProducts(stockFolder + "/products.txt")) return 1;
    if (!inventory.loadInventory(stockFolder + "/inventory.txt")) return 1;
    if (!orders.loadOrders(stockFolder + "/orders.txt")) return 1;

    // Main demo menu loop
    string selection;
    while (true) {
        cout << "\n1 Inventory | 2 Orders | 0 Exit\nSelect> ";
        if (!getline(cin, selection)) {
            break;
        }

        if (selection == "0") {
            break;
        } else if (selection == "1") {
            inventory.showInventoryMenu();
        } else if (selection == "2") {
            orders.showOrderMenu();
        } else {
            cout << "Enter 0, 1, or 2.\n";
        }
    }

    return 0;
}
#endif
