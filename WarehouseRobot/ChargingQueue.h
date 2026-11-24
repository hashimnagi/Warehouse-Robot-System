#pragma once
#include <string>
using namespace std;

struct QueueEntry
{
    string robotID;
    QueueEntry* next;
};

class ChargingQueue
{
private:
    QueueEntry* front;
    QueueEntry* rear;
    int count;

public:
    ChargingQueue();
    ~ChargingQueue();
    void enqueue(string robotID);
    string dequeue();
    bool remove(string robotID);
    bool contains(string robotID);
    string getFront();
    bool isEmpty();
    int getSize();
    void display();
};