#include <iostream>
#include "UndoStack.h"
#include "Common.h"
#include "Location.h"

static string pathStatusText(int status)
{
    if (status == BLOCKED)
    {
        return "BLOCKED";
    }

    return "OPEN";
}

static string robotStatusText(int status)
{
    switch (status)
    {
    case AVAILABLE:
        return "Available";
    case ASSIGNED:
        return "Assigned";
    case WAITING_FOR_CHARGING:
        return "Waiting for Charging";
    case CHARGING_STATUS:
        return "Charging";
    case UNAVAILABLE:
        return "Unavailable";
    }

    return "Unknown";
}

UndoStack::UndoStack()
{
    top = NULL;
    count = 0;
}

UndoStack::~UndoStack()
{
    while (top != NULL)
    {
        pop();
    }
}

bool UndoStack::push(UndoType type, string idOne, string idTwo, int oldValue, int newValue)
{
    UndoRecord* newRecord = new UndoRecord;
    newRecord->type = type;
    newRecord->idOne = idOne;
    newRecord->idTwo = idTwo;
    newRecord->oldValue = oldValue;
    newRecord->newValue = newValue;
    newRecord->next = top;
    top = newRecord;
    count++;
    return true;
}

bool UndoStack::pushQuantityUpdate(string productID, string rackID, int oldQuantity, int newQuantity)
{
    if (productID == "" || rackID == "" || oldQuantity < 0 || newQuantity < 0)
    {
        return false;
    }

    return push(UNDO_QUANTITY, productID, rackID, oldQuantity, newQuantity);
}

bool UndoStack::pushPathwayBlock(string fromID, string toID, int oldStatus)
{
    if (fromID == "" || toID == "" || (oldStatus != OPEN && oldStatus != BLOCKED))
    {
        return false;
    }

    return push(UNDO_PATHWAY, fromID, toID, oldStatus, BLOCKED);
}

bool UndoStack::pushMaintenanceChange(string robotID, int oldStatus)
{
    if (robotID == "" || oldStatus < AVAILABLE || oldStatus > UNAVAILABLE)
    {
        return false;
    }

    return push(UNDO_MAINTENANCE, robotID, "", oldStatus, 0);
}

UndoRecord* UndoStack::peek()
{
    return top;
}

bool UndoStack::pop()
{
    if (top == NULL)
    {
        return false;
    }

    UndoRecord* oldTop = top;
    top = top->next;
    delete oldTop;
    count--;
    return true;
}

bool UndoStack::isEmpty()
{
    return top == NULL;
}

int UndoStack::getCount()
{
    return count;
}

void UndoStack::displayTop()
{
    if (top == NULL)
    {
        cout << "There are no operations to show." << endl;
        return;
    }

    cout << "Most recent operation (" << count << " on the stack): ";

    switch (top->type)
    {
    case UNDO_QUANTITY:
        cout << "manual quantity update, product " << top->idOne << ", rack " << top->idTwo
            << ", old quantity " << top->oldValue << ", new quantity " << top->newValue << endl;
        break;
    case UNDO_PATHWAY:
        cout << "block pathway " << top->idOne << " - " << top->idTwo
            << ", previous status " << pathStatusText(top->oldValue) << endl;
        break;
    case UNDO_MAINTENANCE:
        cout << "robot maintenance change, robot " << top->idOne
            << ", previous status " << robotStatusText(top->oldValue) << endl;
        break;
    }
}