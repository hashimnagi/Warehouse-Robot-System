#include <iostream>
#include "Route.h"

Route::Route()
{
    head = NULL;
    stopCount = 0;
    totalDistance = 0;
}

Route::~Route()
{
    RouteStop* current = head;

    while (current != NULL)
    {
        RouteStop* temp = current->next;
        delete current;
        current = temp;
    }
}

void Route::addStopAtFront(string locationID)
{
    RouteStop* newStop = new RouteStop;
    newStop->locationID = locationID;
    newStop->next = head;
    head = newStop;
    stopCount++;
}

void Route::setTotalDistance(int distance)
{
    totalDistance = distance;
}

RouteStop* Route::getFirstStop()
{
    return head;
}

int Route::getStopCount()
{
    return stopCount;
}

int Route::getTotalDistance()
{
    return totalDistance;
}

void Route::display()
{
    if (head == NULL)
    {
        cout << "No route." << endl;
        return;
    }

    RouteStop* current = head;

    while (current != NULL)
    {
        cout << current->locationID;

        if (current->next != NULL)
        {
            cout << " -> ";
        }

        current = current->next;
    }

    cout << "  (distance " << totalDistance << ")" << endl;
}