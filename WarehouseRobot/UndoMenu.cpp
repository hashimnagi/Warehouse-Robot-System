#include <iostream>
#include "UndoMenu.h"
#include "InventoryManager.h"

//Forward declaration for Person B's inventory manager pointer
//This is set by setUndoInventory() so the undo menu can call revertQuantityUpdate
static InventoryManager* undoInventory = NULL;

//Sets the inventory manager reference so the undo menu can revert quantity updates
void setUndoInventory(InventoryManager* inventory)
{
	undoInventory = inventory;
}

static int askChoice(int maxChoice)
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

//Undoes a pathway block operation by reopening the blocked pathway
static void undoPathway(UndoStack& undoStack, Graph& warehouse, UndoRecord* record)
{
	string fromID = record->idOne;
	string toID = record->idTwo;
	bool wantOpen = (record->oldValue == OPEN);
	string wantText = "BLOCKED";

	if (wantOpen)
	{
		wantText = "OPEN";
	}

	Location* fromLocation = warehouse.findLocation(fromID);

	if (fromLocation == NULL || fromLocation->findEdge(toID) == NULL)
	{
		cout << "The pathway " << fromID << " - " << toID
			<< " no longer exists, so there is nothing to restore. The entry was removed." << endl;
	}
	else if (warehouse.isPathwayOpen(fromID, toID) == wantOpen)
	{
		cout << "The pathway " << fromID << " - " << toID << " is already " << wantText
			<< ". The entry was removed." << endl;
	}
	else
	{
		if (wantOpen)
		{
			warehouse.reopenPathway(fromID, toID);
		}
		else
		{
			warehouse.blockPathway(fromID, toID);
		}

		cout << "Undone: pathway " << fromID << " - " << toID << " is now " << wantText << " again." << endl;
	}

	undoStack.pop();
}

//Undoes a manual quantity update by restoring the old total quantity
static void undoQuantity(UndoStack& undoStack, UndoRecord* record)
{
	if (undoInventory == NULL)
	{
		cout << "Undo for quantity updates is not connected yet. The entry stays on the stack." << endl;
		return;
	}

	string productID = record->idOne;
	string rackID = record->idTwo;
	int oldQuantity = record->oldValue;

	//Check that the rack entry still exists and the restored quantity would not go below reserved
	int reserved = undoInventory->getReservedQty(productID, rackID);

	if (reserved < 0)
	{
		cout << "The rack entry for product " << productID << " in rack " << rackID
			<< " no longer exists. The entry stays on the stack." << endl;
		return;
	}

	if (oldQuantity < reserved)
	{
		cout << "Cannot restore quantity to " << oldQuantity << " because " << reserved
			<< " units are reserved in rack " << rackID
			<< ". The entry stays on the stack." << endl;
		return;
	}

	if (undoInventory->revertQuantityUpdate(productID, rackID, oldQuantity))
	{
		cout << "Undone: product " << productID << " in rack " << rackID
			<< " quantity restored to " << oldQuantity << "." << endl;
		undoStack.pop();
	}
	else
	{
		cout << "Could not revert the quantity update. The entry stays on the stack." << endl;
	}
}

//Undoes a robot maintenance status change
//Person C must provide their robot functions before this can work
static void undoMaintenance(UndoStack& undoStack, UndoRecord* record)
{
	//Person C has not connected their robot module yet
	//When Person C integrates, they should provide a revertMaintenance function
	cout << "Undo for robot maintenance is not connected yet (Person C's module). The entry stays on the stack." << endl;
}

//Processes the undo action for the top entry on the undo stack
static void undoOption(UndoStack& undoStack, Graph& warehouse)
{
	UndoRecord* top = undoStack.peek();

	if (top == NULL)
	{
		cout << "There is nothing to undo." << endl;
		return;
	}

	switch (top->type)
	{
	case UNDO_PATHWAY:
		undoPathway(undoStack, warehouse, top);
		break;
	case UNDO_QUANTITY:
		undoQuantity(undoStack, top);
		break;
	case UNDO_MAINTENANCE:
		undoMaintenance(undoStack, top);
		break;
	}
}

//Discards the top entry from the undo stack without performing any undo action
static void discardOption(UndoStack& undoStack)
{
	if (undoStack.isEmpty())
	{
		cout << "There is nothing to discard." << endl;
		return;
	}

	undoStack.displayTop();
	undoStack.pop();
	cout << "That entry was discarded. The system state was not changed." << endl;
}

//Displays the undo history menu and handles user input
void showUndoMenu(UndoStack& undoStack, Graph& warehouse)
{
	int choice = -1;

	while (choice != 0)
	{
		cout << endl;
		cout << "===== UNDO HISTORY MENU =====" << endl;
		cout << "1. Display the most recent operation" << endl;
		cout << "2. Undo the most recent operation" << endl;
		cout << "3. Discard the most recent operation" << endl;
		cout << "0. Back" << endl;

		choice = askChoice(3);

		switch (choice)
		{
		case 1:
			undoStack.displayTop();
			break;
		case 2:
			undoOption(undoStack, warehouse);
			break;
		case 3:
			discardOption(undoStack);
			break;
		case 0:
			break;
		}
	}
}
