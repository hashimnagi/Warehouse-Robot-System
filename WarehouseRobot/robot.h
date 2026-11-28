#include <iostream>

enum StatusValues {available,assigned,waiting_for_charging,charging,unavailable};

class Robot
{
    public:
    Robot()
    {
        robotID=locationID=batteryLevel=maxCapacity=status=assignedTask=0;
    }
    protected:
    int robotID,
    locationID,
    batteryLevel,
    maxCapacity,    // in kg
    status,
    assignedTask;   // order id or task id, empty if none

};
