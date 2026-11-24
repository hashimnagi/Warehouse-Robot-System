#include <iostream>
#include "UndoMenu.h"

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
        cout << "Undo for quantity updates is not connected yet. The entry stays on the stack." << endl;
        break;
    case UNDO_MAINTENANCE:
        cout << "Undo for robot maintenance is not connected yet. The entry stays on the stack." << endl;
        break;
    }
}

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