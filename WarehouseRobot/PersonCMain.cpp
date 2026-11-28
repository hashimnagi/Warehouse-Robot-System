// This entry point is enabled only by the Person C build, leaving the team's main.cpp intact.
#ifdef WAREHOUSE_PERSON_C_APP
#include "RobotMenus.h"
#include "GraphMenu.h"
#include "WarehouseInput.h"

namespace {
UndoStack* quantityHistory = nullptr;
Graph* distanceGraph = nullptr;

void recordQuantity(const std::string& product, const std::string& rack, int oldQty, int newQty) {
    quantityHistory->pushQuantityUpdate(product, rack, oldQty, newQty);
}
int lookupDistance(const char* from, const char* to) {
    return distanceGraph->getDistance(from, to);
}
}

// All warehouse records come from files. The optional argument selects the data directory.
int main(int argc, char** argv) {
    const std::string data = argc > 1 ? argv[1] : "WarehouseRobot/data";
    Graph warehouse;
    InventoryManager inventory(warehouse);
    OrderManager orders(inventory);
    RobotList fleet(warehouse);
    ChargingNetwork charging;
    UndoStack history;
    distanceGraph = &warehouse;
    quantityHistory = &history;
    inventory.setDistanceLookup(lookupDistance);
    inventory.setQuantityHistory(recordQuantity);

    if (!warehouse.loadLocations(data + "/locations.txt") ||
        !warehouse.loadPathways(data + "/pathways.txt") ||
        !inventory.loadRacks(data + "/racks.txt") ||
        !inventory.loadProducts(data + "/products.txt") ||
        !inventory.loadInventory(data + "/inventory.txt") ||
        !orders.loadOrders(data + "/orders.txt") ||
        !fleet.loadRobots(data + "/robots.txt") ||
        !charging.load(data + "/chargers.txt", warehouse)) {
        std::cerr << "Data loading failed. Pass the correct data directory as the first argument.\n";
        return 1;
    }
    PlanGenerator planner(warehouse, inventory, orders, fleet);
    Executor executor(warehouse, inventory, orders, fleet, planner);
    FleetSupport support(fleet, charging, warehouse, inventory, history);
    std::string line;
    while (true) {
        std::cout << "\nINTELLIGENT WAREHOUSE ROBOT SYSTEM\n"
                  << "1 Warehouse Layout\n2 Product Inventory\n3 Customer Orders\n"
                  << "4 Robot Fleet\n5 Process Order\n6 Execute Order\n"
                  << "7 Charging Management\n8 Undo History\n0 Exit\nSelect> ";
        if (!std::getline(std::cin, line)) break;
        std::istringstream fields(line);
        int choice = -1;
        if (!(fields >> choice) || !endOfRecord(fields)) {
            std::cout << "Enter a number from 0 to 8.\n";
            continue;
        }
        switch (choice) {
        case 0: return 0;
        case 1: showGraphMenu(warehouse, history); break;
        case 2: inventory.showInventoryMenu(); break;
        case 3: orders.showOrderMenu(); break;
        case 4: showRobotMenu(fleet, history); break;
        case 5: showPickingMenu(planner); break;
        case 6: showExecutionMenu(planner, executor); break;
        case 7: showFleetChargingMenu(support, charging); break;
        case 8: showFleetUndoMenu(support, history); break;
        default: std::cout << "Enter a number from 0 to 8.\n"; break;
        }
    }
    return 0;
}
#endif
