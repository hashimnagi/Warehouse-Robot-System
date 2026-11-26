#pragma once
#include "Graph.h"
#include "UndoStack.h"

class InventoryManager;

void setUndoInventory(InventoryManager* inventory);
void showUndoMenu(UndoStack& undoStack, Graph& warehouse);
