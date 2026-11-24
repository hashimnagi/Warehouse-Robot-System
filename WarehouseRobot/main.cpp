#include <iostream>
#include <string>
#include "Graph.h"
#include "UndoStack.h"
#include "GraphMenu.h"
#include "UndoMenu.h"
using namespace std;

static int askMainChoice()
{
    while (true)
    {
        string text;
        cout << "Enter your choice: ";

        if (!getline(cin, text))
        {
            return 0;
        }

        if (text.length() == 1 && text[0] >= '0' && text[0] <= '2')
        {
            return text[0] - '0';
        }

        cout << "Invalid choice. Enter 0, 1 or 2." << endl;
    }
}

int main()
{

    Graph warehouse;
    UndoStack undoStack;
    int choice = -1;
    undoStack.pushQuantityUpdate("P101", "RK01", 10, 15);
    undoStack.pushMaintenanceChange("RB02", AVAILABLE);
    while (choice != 0)
    {
        cout << endl;
        cout << "===== MAIN MENU =====" << endl;
        cout << "1. Warehouse layout" << endl;
        cout << "2. Undo history" << endl;
        cout << "0. Exit" << endl;

        choice = askMainChoice();

        switch (choice)
        {
        case 1:
            showGraphMenu(warehouse, undoStack);
            break;
        case 2:
            showUndoMenu(undoStack, warehouse);
            break;
        case 0:
            break;
        }
    }

    return 0;
}