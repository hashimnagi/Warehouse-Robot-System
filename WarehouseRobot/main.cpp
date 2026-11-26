#include <iostream>
#include "Graph.h"
#include "RackMap.h"
#include "ChargingNetwork.h"
#include "UndoStack.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "GraphMenu.h"
#include "ChargingMenu.h"
#include "UndoMenu.h"

using namespace std;

//Callback function that Person B's InventoryManager calls when a manual quantity update happens
//Records the change on the undo stack so it can be reversed later
static UndoStack* globalUndoStack = NULL;
static Graph* globalWarehouse = NULL;

static void onQuantityChanged(const string& productID, const string& rackID, int oldQty, int newQty)
{
	if (globalUndoStack != NULL)
	{
		globalUndoStack->pushQuantityUpdate(productID, rackID, oldQty, newQty);
	}
}

//Callback function providing shortest-path distance for Person B's InventoryManager
static int warehouseDistanceLookup(const char* fromLocID, const char* toLocID)
{
	if (globalWarehouse != NULL && fromLocID != NULL && toLocID != NULL)
	{
		return globalWarehouse->getDistance(fromLocID, toLocID);
	}
	return -1;
}

//Loads all data files for the warehouse system
//Returns true only if all files loaded without errors
static bool loadAllData(Graph& warehouse, RackLocationMap& rackMap,
	ChargingNetwork& network, InventoryManager& inventory, OrderManager& orders)
{
	cout << "Loading warehouse data..." << endl;

	if (!warehouse.loadLocations("data\\locations.txt"))
	{
		cout << "Failed to load locations." << endl;
		return false;
	}

	if (!warehouse.loadPathways("data\\pathways.txt"))
	{
		cout << "Failed to load pathways." << endl;
		return false;
	}

	if (!rackMap.load("data\\racks.txt", warehouse))
	{
		cout << "Failed to load rack map." << endl;
		return false;
	}

	if (!network.load("data\\chargers.txt", warehouse))
	{
		cout << "Failed to load charging stations." << endl;
		return false;
	}

	if (!inventory.loadRacks("data\\racks.txt"))
	{
		cout << "Failed to load inventory racks." << endl;
		return false;
	}

	if (!inventory.loadProducts("data\\products.txt"))
	{
		cout << "Failed to load products." << endl;
		return false;
	}

	if (!inventory.loadInventory("data\\inventory.txt"))
	{
		cout << "Failed to load inventory." << endl;
		return false;
	}

	if (!orders.loadOrders("data\\orders.txt"))
	{
		cout << "Failed to load orders." << endl;
		return false;
	}

	cout << endl;
	cout << "All data loaded successfully." << endl;
	return true;
}

//Reads the user's menu choice (a single digit between 0 and maxChoice)
static int askMainChoice(int maxChoice)
{
	while (true)
	{
		string text;
		cout << "Enter your choice: ";

		if (!getline(cin, text))
		{
			return 0;
		}

		if (text.length() == 1 && text[0] >= '0' && text[0] <= '0' + maxChoice)
		{
			return text[0] - '0';
		}

		cout << "Invalid choice. Enter a number from 0 to " << maxChoice << "." << endl;
	}
}

int main()
{
	//Create all the core objects for the system
	Graph warehouse;
	RackLocationMap rackMap;
	ChargingNetwork network;
	UndoStack undoStack;
	InventoryManager inventory(warehouse);
	OrderManager orders(inventory);

	//Set up the global pointers so callbacks can reach them
	globalUndoStack = &undoStack;
	globalWarehouse = &warehouse;

	//Register distance lookup callback so inventory uses Person A's Dijkstra
	inventory.setDistanceLookup(warehouseDistanceLookup);

	//Register the quantity-change callback with the inventory manager
	//so that manual stock adjustments are automatically pushed onto the undo stack
	inventory.setQuantityHistory(onQuantityChanged);
	setUndoInventory(&inventory);

	//Load all data files at startup
	if (!loadAllData(warehouse, rackMap, network, inventory, orders))
	{
		cout << "Some data files could not be loaded. The system may have incomplete data." << endl;
	}

	int choice = -1;

	while (choice != 0)
	{
		cout << endl;
		cout << "========================================" << endl;
		cout << " INTELLIGENT WAREHOUSE ROBOT SYSTEM" << endl;
		cout << "========================================" << endl;
		cout << "1. Warehouse Layout" << endl;
		cout << "2. Product Inventory" << endl;
		cout << "3. Customer Orders" << endl;
		cout << "4. Robot Fleet" << endl;
		cout << "5. Process Order" << endl;
		cout << "6. Execute Order" << endl;
		cout << "7. Charging Management" << endl;
		cout << "8. Undo History" << endl;
		cout << "0. Exit" << endl;

		choice = askMainChoice(8);

		switch (choice)
		{
		case 1:
			//Person A's warehouse layout menu (Module 1)
			showGraphMenu(warehouse, undoStack);
			break;
		case 2:
			//Person B's inventory menu (Module 2)
			inventory.showInventoryMenu();
			break;
		case 3:
			//Person B's order management menu (Module 3)
			orders.showOrderMenu();
			break;
		case 4:
			//Person C's robot fleet menu (Module 4)
			cout << "Robot Fleet management is not connected yet (Person C's module)." << endl;
			break;
		case 5:
			//Person C's picking plan menu (Module 5)
			cout << "Process Order is not connected yet (Person C's module)." << endl;
			break;
		case 6:
			//Person C's execution menu (Module 6)
			cout << "Execute Order is not connected yet (Person C's module)." << endl;
			break;
		case 7:
			//Person A's charging management menu (Module 7)
			showChargingMenu(network, warehouse, undoStack);
			break;
		case 8:
			//Person A's undo history menu (Module 8)
			showUndoMenu(undoStack, warehouse);
			break;
		case 0:
			cout << "Exiting the system. Goodbye." << endl;
			break;
		}
	}

	return 0;
}
