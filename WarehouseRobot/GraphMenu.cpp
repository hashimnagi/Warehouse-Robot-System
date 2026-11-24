#include <iostream>
#include <sstream>
#include "GraphMenu.h"
#include "Route.h"

static const int ANY_PATHWAY = 0;
static const int OPEN_PATHWAY = 1;
static const int BLOCKED_PATHWAY = 2;

static string trimText(string text)
{
    size_t first = text.find_first_not_of(" \t");

    if (first == string::npos)
    {
        return "";
    }

    size_t last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}

static bool readLine(string prompt, string& text)
{
    cout << prompt;

    if (!getline(cin, text))
    {
        text = "";
        return false;
    }

    text = trimText(text);
    return true;
}

static bool isDigitsOnly(string text, size_t maxLength)
{
    if (text == "" || text.length() > maxLength)
    {
        return false;
    }

    for (size_t i = 0; i < text.length(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }
    }

    return true;
}

static int askMenuChoice(int maxChoice)
{
    while (true)
    {
        string text;

        if (!readLine("Enter your choice: ", text))
        {
            return 0;
        }

        if (isDigitsOnly(text, 2))
        {
            int value;
            stringstream reader(text);
            reader >> value;

            if (value <= maxChoice)
            {
                return value;
            }
        }

        cout << "Invalid choice. Enter a number from 0 to " << maxChoice << "." << endl;
    }
}

static int askNumberInRange(string prompt, int maxValue)
{
    while (true)
    {
        string text;

        if (!readLine(prompt, text) || text == "0")
        {
            return 0;
        }

        if (isDigitsOnly(text, 6))
        {
            int value;
            stringstream reader(text);
            reader >> value;

            if (value >= 1 && value <= maxValue)
            {
                return value;
            }
        }

        cout << "Invalid input. Enter a number from 1 to " << maxValue << ", or 0 to cancel." << endl;
    }
}

static bool isValidLocationID(string id)
{
    if (id.length() < 2 || id[0] != 'L')
    {
        return false;
    }

    for (size_t i = 1; i < id.length(); i++)
    {
        if (id[i] < '0' || id[i] > '9')
        {
            return false;
        }
    }

    return true;
}

static string typeName(LocationType type)
{
    switch (type)
    {
    case RACK:
        return "RACK";
    case PACKING:
        return "PACKING";
    case CHARGING:
        return "CHARGING";
    }

    return "UNKNOWN";
}

static string askNewID(Graph& warehouse, string prompt)
{
    while (true)
    {
        string id;

        if (!readLine(prompt, id) || id == "0")
        {
            return "";
        }

        if (!isValidLocationID(id))
        {
            cout << "Invalid ID. Use a capital L followed by digits, for example L16, or enter 0 to cancel." << endl;
            continue;
        }

        if (warehouse.findLocation(id) != NULL)
        {
            cout << "Location " << id << " already exists. Enter a different ID, or 0 to cancel." << endl;
            continue;
        }

        return id;
    }
}

static string askExistingID(Graph& warehouse, string prompt)
{
    while (true)
    {
        string id;

        if (!readLine(prompt, id) || id == "0")
        {
            return "";
        }

        if (!isValidLocationID(id))
        {
            cout << "Invalid ID. Use a capital L followed by digits, for example L01, or enter 0 to cancel." << endl;
            continue;
        }

        if (warehouse.findLocation(id) == NULL)
        {
            cout << "Location " << id << " does not exist. Try again, or enter 0 to cancel." << endl;
            continue;
        }

        return id;
    }
}

static bool askLocationType(LocationType& type)
{
    int choice = askNumberInRange("Type (1 = RACK, 2 = PACKING, 3 = CHARGING, 0 to cancel): ", 3);

    switch (choice)
    {
    case 1:
        type = RACK;
        return true;
    case 2:
        type = PACKING;
        return true;
    case 3:
        type = CHARGING;
        return true;
    }

    return false;
}

static string askName(string prompt)
{
    while (true)
    {
        string name;

        if (!readLine(prompt, name) || name == "0")
        {
            return "";
        }

        if (name == "" || name.find_first_of(" \t") != string::npos)
        {
            cout << "Invalid name. Use one word with no spaces, for example Rack_Zone_E1, or enter 0 to cancel." << endl;
            continue;
        }

        return name;
    }
}

static bool askTwoLocations(Graph& warehouse, string& fromID, string& toID)
{
    fromID = askExistingID(warehouse, "First location ID (0 to cancel): ");

    if (fromID == "")
    {
        cout << "Cancelled." << endl;
        return false;
    }

    toID = askExistingID(warehouse, "Second location ID (0 to cancel): ");

    if (toID == "")
    {
        cout << "Cancelled." << endl;
        return false;
    }

    return true;
}

static bool askExistingPathway(Graph& warehouse, string& fromID, string& toID, int requiredState)
{
    while (true)
    {
        if (!askTwoLocations(warehouse, fromID, toID))
        {
            return false;
        }

        if (warehouse.findLocation(fromID)->findEdge(toID) == NULL)
        {
            cout << "There is no pathway between " << fromID << " and " << toID << ". Enter a different pair." << endl;
            continue;
        }

        if (requiredState == OPEN_PATHWAY && !warehouse.isPathwayOpen(fromID, toID))
        {
            cout << "That pathway is already blocked. Enter a different pair." << endl;
            continue;
        }

        if (requiredState == BLOCKED_PATHWAY && warehouse.isPathwayOpen(fromID, toID))
        {
            cout << "That pathway is already open. Enter a different pair." << endl;
            continue;
        }

        return true;
    }
}

static void loadOption(Graph& warehouse)
{
    if (warehouse.getLocationCount() > 0)
    {
        cout << "Data is already loaded." << endl;
        return;
    }

    warehouse.loadLocations("data\\locations.txt");
    warehouse.loadPathways("data\\pathways.txt");
}

static void addLocationOption(Graph& warehouse)
{
    string id = askNewID(warehouse, "New location ID, for example L16 (0 to cancel): ");

    if (id == "")
    {
        cout << "Cancelled." << endl;
        return;
    }

    LocationType type = RACK;

    if (!askLocationType(type))
    {
        cout << "Cancelled." << endl;
        return;
    }

    string name = askName("Name, one word, for example Rack_Zone_E1 (0 to cancel): ");

    if (name == "")
    {
        cout << "Cancelled." << endl;
        return;
    }

    if (warehouse.addLocation(id, type, name))
    {
        cout << "Location " << id << " added." << endl;
    }
    else
    {
        cout << "Could not add the location." << endl;
    }
}

static void addPathwayOption(Graph& warehouse)
{
    string fromID;
    string toID;

    while (true)
    {
        if (!askTwoLocations(warehouse, fromID, toID))
        {
            return;
        }

        if (fromID == toID)
        {
            cout << "A pathway cannot connect a location to itself. Enter two different locations." << endl;
            continue;
        }

        if (warehouse.findLocation(fromID)->findEdge(toID) != NULL)
        {
            cout << "A pathway between " << fromID << " and " << toID << " already exists. Enter a different pair." << endl;
            continue;
        }

        break;
    }

    int distance = askNumberInRange("Distance, a whole number above 0 (0 to cancel): ", 999999);

    if (distance == 0)
    {
        cout << "Cancelled." << endl;
        return;
    }

    if (warehouse.addPathway(fromID, toID, distance))
    {
        cout << "Pathway " << fromID << " - " << toID << " added with distance " << distance << "." << endl;
    }
    else
    {
        cout << "Could not add the pathway." << endl;
    }
}

static void removePathwayOption(Graph& warehouse)
{
    string fromID;
    string toID;

    if (!askExistingPathway(warehouse, fromID, toID, ANY_PATHWAY))
    {
        return;
    }

    warehouse.removePathway(fromID, toID);
    cout << "Pathway " << fromID << " - " << toID << " removed." << endl;
}

static void blockPathwayOption(Graph& warehouse, UndoStack& undoStack)
{
    string fromID;
    string toID;

    if (!askExistingPathway(warehouse, fromID, toID, OPEN_PATHWAY))
    {
        return;
    }

    warehouse.blockPathway(fromID, toID);
    undoStack.pushPathwayBlock(fromID, toID, OPEN);
    cout << "Pathway " << fromID << " - " << toID << " is now BLOCKED." << endl;
}

static void reopenPathwayOption(Graph& warehouse)
{
    string fromID;
    string toID;

    if (!askExistingPathway(warehouse, fromID, toID, BLOCKED_PATHWAY))
    {
        return;
    }

    warehouse.reopenPathway(fromID, toID);
    cout << "Pathway " << fromID << " - " << toID << " is now OPEN." << endl;
}

static void searchOption(Graph& warehouse)
{
    string id = askExistingID(warehouse, "Location ID to search (0 to cancel): ");

    if (id == "")
    {
        cout << "Cancelled." << endl;
        return;
    }

    Location* location = warehouse.findLocation(id);

    cout << "ID: " << location->getID() << endl;
    cout << "Type: " << typeName(location->getType()) << endl;
    cout << "Name: " << location->getName() << endl;
    cout << "Pathways: " << location->countEdges() << endl;

    Edge* edge = location->getFirstEdge();

    while (edge != NULL)
    {
        cout << "    -> " << edge->destination->getID() << "  distance " << edge->distance;

        if (edge->status == BLOCKED)
        {
            cout << "  BLOCKED" << endl;
        }
        else
        {
            cout << "  OPEN" << endl;
        }

        edge = edge->next;
    }
}

static void connectedOption(Graph& warehouse)
{
    string fromID;
    string toID;

    if (!askTwoLocations(warehouse, fromID, toID))
    {
        return;
    }

    if (warehouse.isConnected(fromID, toID))
    {
        cout << fromID << " and " << toID << " are connected." << endl;
    }
    else
    {
        cout << fromID << " and " << toID << " are not connected (no open route)." << endl;
    }
}

static void routeOption(Graph& warehouse)
{
    string fromID;
    string toID;

    if (!askTwoLocations(warehouse, fromID, toID))
    {
        return;
    }

    Route* route = warehouse.getShortestRoute(fromID, toID);

    if (route == NULL)
    {
        cout << "No route exists from " << fromID << " to " << toID << "." << endl;
        return;
    }

    cout << "Shortest route: ";
    route->display();
    delete route;
}

void showGraphMenu(Graph& warehouse, UndoStack& undoStack)
{
    int choice = -1;

    while (choice != 0)
    {
        cout << endl;
        cout << "===== WAREHOUSE LAYOUT MENU =====" << endl;
        cout << "1. Load locations and pathways from files" << endl;
        cout << "2. Display all locations and pathways" << endl;
        cout << "3. Add a location" << endl;
        cout << "4. Add a pathway" << endl;
        cout << "5. Remove a pathway" << endl;
        cout << "6. Block a pathway" << endl;
        cout << "7. Reopen a pathway" << endl;
        cout << "8. Search a location by ID" << endl;
        cout << "9. Check if two locations are connected (BFS)" << endl;
        cout << "10. Find the shortest route (Dijkstra)" << endl;
        cout << "0. Back" << endl;

        choice = askMenuChoice(10);

        switch (choice)
        {
        case 1:
            loadOption(warehouse);
            break;
        case 2:
            warehouse.displayAll();
            break;
        case 3:
            addLocationOption(warehouse);
            break;
        case 4:
            addPathwayOption(warehouse);
            break;
        case 5:
            removePathwayOption(warehouse);
            break;
        case 6:
            blockPathwayOption(warehouse, undoStack);
            break;
        case 7:
            reopenPathwayOption(warehouse);
            break;
        case 8:
            searchOption(warehouse);
            break;
        case 9:
            connectedOption(warehouse);
            break;
        case 10:
            routeOption(warehouse);
            break;
        case 0:
            break;
        }
    }
}