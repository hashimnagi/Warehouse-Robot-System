#include "RobotMenus.h"
#include "WarehouseInput.h"

namespace {
// Line-based input never leaves a failed numeric extraction in the console stream.
bool readCommand(const char* prompt, std::istringstream& fields, int& choice) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    fields.str(line);
    fields.clear();
    if (!(fields >> choice)) choice = -1;
    return true;
}
void result(bool success, const std::string& reason = "") {
    std::cout << (success ? "OK. " : "Rejected. ") << reason << '\n';
}
}

// Fleet administration uses validated commands and records maintenance changes.
void showRobotMenu(RobotList& fleet, UndoStack& history) {
    std::cout << "\nRobot Fleet (one command per line)\n"
              << "1                     Display all robots\n"
              << "2                     Display available robots\n"
              << "3 robotID             Search robot\n"
              << "4 id type loc battery capacity  Register (PICKER/CARRIER/HEAVY_DUTY)\n"
              << "5 robotID battery     Update battery (0-100)\n"
              << "6 robotID 0|1         Maintenance (1 unavailable, 0 available)\n"
              << "7 filename            Load robots from file\n"
              << "0                     Back\n";
    std::istringstream fields;
    int choice;
    while (readCommand("Fleet> ", fields, choice)) {
        std::string id, typeText, location;
        double battery = 0, capacity = 0;
        int maintenance = -1;
        bool success = false;
        if (choice == 0 && endOfRecord(fields)) return;
        if (choice == 1 && endOfRecord(fields)) { fleet.displayAll(); continue; }
        if (choice == 2 && endOfRecord(fields)) { fleet.displayAll(true); continue; }
        if (choice == 3 && (fields >> id) && endOfRecord(fields)) {
            Robot* robot = fleet.searchRobot(id);
            if (robot != nullptr) robot->display(std::cout);
            else result(false, "Unknown robot.");
            continue;
        }
        if (choice == 4 && (fields >> id >> typeText >> location >> battery >> capacity) && endOfRecord(fields)) {
            RobotType type = PICKER;
            bool valid = true;
            if (typeText == "PICKER") type = PICKER;
            else if (typeText == "CARRIER") type = CARRIER;
            else if (typeText == "HEAVY_DUTY") type = HEAVY_DUTY;
            else valid = false;
            success = valid && fleet.registerRobot(id, type, location, battery, capacity);
        } else if (choice == 5 && (fields >> id >> battery) && endOfRecord(fields)) {
            success = fleet.updateBattery(id, battery);
        } else if (choice == 6 && (fields >> id >> maintenance) && endOfRecord(fields) &&
                   (maintenance == 0 || maintenance == 1)) {
            success = fleet.setMaintenance(id, maintenance == 1, &history);
        } else if (choice == 7 && (fields >> id) && endOfRecord(fields)) {
            success = fleet.loadRobots(id);
        }
        result(success, success ? "Fleet updated." : "Check command, IDs, status and type limits.");
    }
}

// Preview is read-only. Only an explicit yes transfers ownership and reserves stock.
void showPickingMenu(PlanGenerator& planner) {
    std::cout << "\nProcess Order\n1 Preview next feasible pending order\n"
              << "2 Preview next order allowing partial stock\n0 Back\n";
    std::istringstream fields;
    int choice;
    while (readCommand("Picking> ", fields, choice)) {
        if (choice == 0 && endOfRecord(fields)) return;
        if ((choice != 1 && choice != 2) || !endOfRecord(fields)) {
            result(false, "Use 0, 1 or 2.");
            continue;
        }
        std::string reason;
        PickingPlan* preview = planner.previewNext(choice == 2, reason);
        if (!reason.empty()) std::cout << reason << '\n';
        if (preview == nullptr) continue;
        preview->display();
        std::cout << "Confirm reservation and assignment? (yes/no): ";
        std::string answer;
        if (!std::getline(std::cin, answer)) { delete preview; return; }
        if (answer == "yes") {
            bool confirmed = planner.confirm(preview, reason);
            result(confirmed, reason);
            if (!confirmed) delete preview;
        } else {
            delete preview;
            std::cout << "Preview discarded; nothing reserved.\n";
        }
    }
}

// A trip is executed only by its dedicated command; assignment can remain deferred.
void showExecutionMenu(PlanGenerator& planner, Executor& executor) {
    std::cout << "\nExecute Order\n1 orderID Display assigned plan\n2 orderID Execute next trip\n"
              << "3 orderID Recalculate routes\n4 orderID Cancel Assignment and requeue\n"
              << "0 Back (keep assignments for later execution)\n";
    std::istringstream fields;
    int choice;
    while (readCommand("Execution> ", fields, choice)) {
        if (choice == 0 && endOfRecord(fields)) return;
        std::string id, reason;
        if (!(fields >> id) || !endOfRecord(fields)) { result(false, "Enter command and orderID."); continue; }
        if (choice == 1) {
            PickingPlan* plan = planner.findPlan(id);
            if (plan != nullptr) plan->display();
            else result(false, "No assigned plan.");
        } else if (choice == 2) result(executor.executeTrip(id, reason), reason);
        else if (choice == 3) result(executor.recalculateRoute(id, reason), reason);
        else if (choice == 4) result(planner.cancelAssignment(id, reason), reason);
        else result(false, "Unknown command.");
    }
}

// The adapter uses actual fleet records when calling Person A's charging network.
void showFleetChargingMenu(FleetSupport& support, ChargingNetwork& network) {
    std::cout << "\nCharging\n1 Display stations\n2 Display robots below 20%\n"
              << "3 robotID Request charging\n4 robotID Complete charge\n5 robotID Leave queue\n0 Back\n";
    std::istringstream fields;
    int choice;
    while (readCommand("Charging> ", fields, choice)) {
        if (choice == 0 && endOfRecord(fields)) return;
        if (choice == 1 && endOfRecord(fields)) { network.displayAll(); continue; }
        if (choice == 2 && endOfRecord(fields)) { support.displayNeedsCharging(); continue; }
        std::string id, reason;
        if (!(fields >> id) || !endOfRecord(fields)) { result(false, "Enter command and robotID."); continue; }
        if (choice == 3) result(support.requestCharging(id, reason), reason);
        else if (choice == 4) result(support.completeCharging(id, reason), reason);
        else if (choice == 5) result(support.removeFromChargingQueue(id));
        else result(false, "Unknown command.");
    }
}

// Maintenance restoration is supplied here without changing Person A's menu files.
void showFleetUndoMenu(FleetSupport& support, UndoStack& history) {
    std::cout << "\nUndo History\n1 Display latest\n2 Undo latest\n3 Discard latest\n0 Back\n";
    std::istringstream fields;
    int choice;
    while (readCommand("Undo> ", fields, choice)) {
        if (!endOfRecord(fields)) { result(false, "Enter a single number."); continue; }
        if (choice == 0) return;
        std::string reason;
        if (choice == 1) history.displayTop();
        else if (choice == 2) result(support.undo(reason), reason);
        else if (choice == 3) result(history.pop());
        else result(false, "Unknown command.");
    }
}
