#pragma once
#include <string>
#include "Graph.h"
using namespace std;

class QueueNode
{
public:
    Location* location;
    QueueNode* next;
};

class LocationQueue
{
private:
    QueueNode* front;
    QueueNode* rear;

public:
    LocationQueue();
    ~LocationQueue();
    void enqueue(Location* location);
    Location* dequeue();
    bool isEmpty();
};

