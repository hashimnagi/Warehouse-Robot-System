#pragma once
#include "Executor.h"
#include "FleetSupport.h"

void showRobotMenu(RobotList& fleet, UndoStack& history);
void showPickingMenu(PlanGenerator& planner);
void showExecutionMenu(PlanGenerator& planner, Executor& executor);
void showFleetChargingMenu(FleetSupport& support, ChargingNetwork& network);
void showFleetUndoMenu(FleetSupport& support, UndoStack& history);
