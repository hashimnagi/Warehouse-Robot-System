#include <iostream>
#include <sstream>
#include <iomanip>
#include "ChargingMenu.h"
#include "Common.h"

//Trims whitespace from both ends of the given text
static string trimInput(string text)
{
	size_t first = text.find_first_not_of(" \t");

	if (first == string::npos)
	{
		return "";
	}

	size_t last = text.find_last_not_of(" \t");
	return text.substr(first, last - first + 1);
}

//Reads one line from the console and trims it
static bool readInputLine(string prompt, string& text)
{
	cout << prompt;

	if (!getline(cin, text))
	{
		text = "";
		return false;
	}

	text = trimInput(text);
	return true;
}

//Checks whether a string contains only digits (up to a certain length)
static bool onlyDigits(string text, size_t maxLength)
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

//Asks the user for a menu choice between 0 and maxChoice
static int askChargingMenuChoice(int maxChoice)
{
	while (true)
	{
		string text;

		if (!readInputLine("Enter your choice: ", text))
		{
			return 0;
		}

		if (onlyDigits(text, 2))
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

//Asks the user for a valid robot ID (RB followed by digits)
static bool isValidRobotID(string id)
{
	if (id.length() < 3 || id[0] != 'R' || id[1] != 'B')
	{
		return false;
	}

	for (size_t i = 2; i < id.length(); i++)
	{
		if (id[i] < '0' || id[i] > '9')
		{
			return false;
		}
	}

	return true;
}

//Asks the user for a valid charging station ID (C followed by digits)
static bool isValidChargerID(string id)
{
	if (id.length() < 2 || id[0] != 'C')
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

//Asks the user for a valid location ID (L followed by digits)
static bool isValidLocID(string id)
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

//Converts a RobotStatus enum value to a readable string
static string robotStatusName(int status)
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

//Displays all charging stations with their slots and queues
static void displayAllStations(ChargingNetwork& network)
{
	if (network.getStationCount() == 0)
	{
		cout << "No charging stations are loaded." << endl;
		return;
	}

	network.displayAll();
}

//Checks whether a robot needs charging according to the battery < 20% rule
static void checkNeedsChargingOption()
{
	string robotID;

	if (!readInputLine("Robot ID (0 to cancel): ", robotID) || robotID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidRobotID(robotID))
	{
		cout << "Invalid robot ID. Use RB followed by digits, for example RB01." << endl;
		return;
	}

	string batteryText;

	if (!readInputLine("Enter robot's current battery level (0.0 - 100.0): ", batteryText) || batteryText == "")
	{
		cout << "Cancelled." << endl;
		return;
	}

	double battery;
	stringstream reader(batteryText);
	reader >> battery;

	if (reader.fail() || battery < 0.0 || battery > 100.0)
	{
		cout << "Invalid battery level. Must be between 0.0 and 100.0." << endl;
		return;
	}

	cout << fixed << setprecision(1);
	cout << "Robot " << robotID << " battery level: " << battery << "%" << endl;

	if (battery < 20.0)
	{
		cout << "CRITICAL: Battery is below 20.0%. Robot NEEDS CHARGING." << endl;
		cout << "Recommended status: WAITING_FOR_CHARGING. Robot should be routed to a charging station." << endl;
	}
	else
	{
		cout << "NORMAL: Battery is sufficient (>= 20.0%). Robot is ready for assignments." << endl;
	}
}

//Finds the nearest reachable station for a robot at a given location
static void findNearestStation(ChargingNetwork& network, Graph& warehouse)
{
	string locationID;

	if (!readInputLine("Robot's current location ID (0 to cancel): ", locationID) || locationID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidLocID(locationID))
	{
		cout << "Invalid location ID. Use L followed by digits, for example L01." << endl;
		return;
	}

	if (warehouse.findLocation(locationID) == NULL)
	{
		cout << "Location " << locationID << " does not exist." << endl;
		return;
	}

	string batteryText;

	if (!readInputLine("Robot's current battery level (0-100): ", batteryText) || batteryText == "")
	{
		cout << "Cancelled." << endl;
		return;
	}

	double battery;
	stringstream batteryReader(batteryText);
	batteryReader >> battery;

	if (batteryReader.fail() || battery < 0 || battery > 100)
	{
		cout << "Invalid battery level. Enter a number from 0 to 100." << endl;
		return;
	}

	string rateText;

	if (!readInputLine("Robot's battery consumption rate (for example 0.20): ", rateText) || rateText == "")
	{
		cout << "Cancelled." << endl;
		return;
	}

	double rate;
	stringstream rateReader(rateText);
	rateReader >> rate;

	if (rateReader.fail() || rate <= 0)
	{
		cout << "Invalid rate. Enter a positive number." << endl;
		return;
	}

	StationChoice choice = network.findBestStation(warehouse, locationID, battery, rate);

	if (choice.station == NULL)
	{
		cout << choice.reason << endl;
		return;
	}

	cout << "Best station: " << choice.station->getID()
		<< " at " << choice.station->getLocationID()
		<< ", distance " << choice.distance;

	if (choice.hasFreeSlot)
	{
		cout << ", free slot available" << endl;
	}
	else
	{
		cout << ", no free slot, robot will join the waiting queue" << endl;
	}
}

//Checks if a specific charging station has an available slot
static void checkSlotAvailableOption(ChargingNetwork& network)
{
	string stationID;

	if (!readInputLine("Station ID (0 to cancel): ", stationID) || stationID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidChargerID(stationID))
	{
		cout << "Invalid station ID. Use C followed by digits, for example C01." << endl;
		return;
	}

	ChargingStation* station = network.findStation(stationID);

	if (station == NULL)
	{
		cout << "Station " << stationID << " does not exist." << endl;
		return;
	}

	cout << "Station " << stationID << " at location " << station->getLocationID() << ":" << endl;
	cout << "  Total slots: " << station->getTotalSlots() << endl;
	cout << "  Free slots:  " << station->getFreeSlots() << endl;
	cout << "  Queue length: " << station->getQueue().getSize() << " robot(s)" << endl;

	if (station->hasFreeSlot())
	{
		cout << "  Status: FREE SLOT AVAILABLE for charging." << endl;
	}
	else
	{
		cout << "  Status: FULL. Incoming robots must wait in queue." << endl;
	}
}

//Sends a robot to a charging station (starts charging or joins the queue)
static void sendRobotToCharge(ChargingNetwork& network, Graph& warehouse)
{
	string robotID;

	if (!readInputLine("Robot ID (0 to cancel): ", robotID) || robotID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidRobotID(robotID))
	{
		cout << "Invalid robot ID. Use RB followed by digits, for example RB01." << endl;
		return;
	}

	string stationID;

	if (!readInputLine("Station ID (0 to cancel): ", stationID) || stationID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidChargerID(stationID))
	{
		cout << "Invalid station ID. Use C followed by digits, for example C01." << endl;
		return;
	}

	ChargingStation* station = network.findStation(stationID);

	if (station == NULL)
	{
		cout << "Station " << stationID << " does not exist." << endl;
		return;
	}

	int result = network.requestCharging(robotID, station);

	switch (result)
	{
	case CHARGE_STARTED:
		cout << "Robot " << robotID << " started charging at station " << stationID << "." << endl;
		cout << "Status changed to CHARGING. Location moved to " << station->getLocationID() << "." << endl;
		break;
	case CHARGE_QUEUED:
		cout << "No free slot. Robot " << robotID << " added to the waiting queue at station " << stationID << "." << endl;
		cout << "Status changed to WAITING_FOR_CHARGING." << endl;
		break;
	case CHARGE_REFUSED:
		cout << "Request refused. Robot " << robotID << " may already be charging or in a queue." << endl;
		break;
	}
}

//Completes charging for a robot and auto-starts the next robot in the queue
static void completeChargingOption(ChargingNetwork& network)
{
	string robotID;

	if (!readInputLine("Robot ID that finished charging (0 to cancel): ", robotID) || robotID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidRobotID(robotID))
	{
		cout << "Invalid robot ID. Use RB followed by digits, for example RB01." << endl;
		return;
	}

	string startedRobotID;
	bool result = network.completeCharging(robotID, startedRobotID);

	if (!result)
	{
		cout << "Robot " << robotID << " is not currently charging at any station." << endl;
		return;
	}

	cout << "Robot " << robotID << " has completed charging." << endl;
	cout << "Battery restored to 100.0%. Status returned to AVAILABLE." << endl;

	if (startedRobotID != "")
	{
		cout << "Next robot in queue (" << startedRobotID << ") has automatically started charging in the freed slot." << endl;
		cout << "Robot " << startedRobotID << " status changed to CHARGING." << endl;
	}
	else
	{
		cout << "No robots were waiting in the queue. The slot is now free." << endl;
	}
}

//Removes a robot from a charging station's waiting queue
static void removeFromQueueOption(ChargingNetwork& network)
{
	string robotID;

	if (!readInputLine("Robot ID to remove from the queue (0 to cancel): ", robotID) || robotID == "0")
	{
		cout << "Cancelled." << endl;
		return;
	}

	if (!isValidRobotID(robotID))
	{
		cout << "Invalid robot ID. Use RB followed by digits, for example RB01." << endl;
		return;
	}

	if (network.removeFromQueue(robotID))
	{
		cout << "Robot " << robotID << " removed from the charging queue. Status set to AVAILABLE." << endl;
	}
	else
	{
		cout << "Robot " << robotID << " is not in any charging queue." << endl;
	}
}

//Displays the charging management menu and handles user input
void showChargingMenu(ChargingNetwork& network, Graph& warehouse, UndoStack& undoStack)
{
	int choice = -1;

	while (choice != 0)
	{
		cout << endl;
		cout << "===== CHARGING MANAGEMENT MENU =====" << endl;
		cout << "1. Display all charging stations" << endl;
		cout << "2. Check if a robot needs charging (battery < 20%)" << endl;
		cout << "3. Find nearest reachable station (Dijkstra)" << endl;
		cout << "4. Check if a station has a free slot" << endl;
		cout << "5. Send robot to charge (start charging or queue)" << endl;
		cout << "6. Complete charging for a robot (100% & auto-start next)" << endl;
		cout << "7. Remove a robot from the charging queue" << endl;
		cout << "0. Back" << endl;

		choice = askChargingMenuChoice(7);

		switch (choice)
		{
		case 1:
			displayAllStations(network);
			break;
		case 2:
			checkNeedsChargingOption();
			break;
		case 3:
			findNearestStation(network, warehouse);
			break;
		case 4:
			checkSlotAvailableOption(network);
			break;
		case 5:
			sendRobotToCharge(network, warehouse);
			break;
		case 6:
			completeChargingOption(network);
			break;
		case 7:
			removeFromQueueOption(network);
			break;
		case 0:
			break;
		}
	}
}
