#pragma once
#include <string>
using namespace std;

struct RouteStop
{
    string locationID;
    RouteStop* next;
};

class Route
{
private:
    RouteStop* head;
    int stopCount;
    int totalDistance;

public:
    Route();
    ~Route();
    void addStopAtFront(string locationID);
    void setTotalDistance(int distance);
    RouteStop* getFirstStop();
    int getStopCount();
    int getTotalDistance();
    void display();
};