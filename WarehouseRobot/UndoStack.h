#pragma once
#include <string>
using namespace std;

enum UndoType
{
    UNDO_QUANTITY,
    UNDO_PATHWAY,
    UNDO_MAINTENANCE
};

struct UndoRecord
{
    UndoType type;
    string idOne;
    string idTwo;
    int oldValue;
    int newValue;
    UndoRecord* next;
};

class UndoStack
{
private:
    UndoRecord* top;
    int count;
    bool push(UndoType type, string idOne, string idTwo, int oldValue, int newValue);

public:
    UndoStack();
    ~UndoStack();

    bool pushQuantityUpdate(string productID, string rackID, int oldQuantity, int newQuantity);
    bool pushPathwayBlock(string fromID, string toID, int oldStatus);
    bool pushMaintenanceChange(string robotID, int oldStatus);

    UndoRecord* peek();
    bool pop();
    bool isEmpty();
    int getCount();
    void displayTop();
};