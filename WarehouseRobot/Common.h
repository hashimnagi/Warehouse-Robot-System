#pragma once
//ID formats that we has to be used like:
//L01, RK04, P101, RB01, C01, O101, CU12

enum LocationType
{
    RACK,
    PACKING,
    CHARGING
};

enum RobotType
{
    PICKER,
    CARRIER,
    HEAVY_DUTY
};

enum RobotStatus
{
    AVAILABLE,
    ASSIGNED,
    WAITING_FOR_CHARGING,
    CHARGING_STATUS,
    UNAVAILABLE
};

enum OrderStatus
{
    PENDING,
    RESERVED,
    ASSIGNED_ORDER,
    PARTIALLY_FULFILLED,
    COMPLETED,
    CANCELLED
};