#include <iostream>
#include "ChargingQueue.h"

ChargingQueue::ChargingQueue()
{
    front = NULL;
    rear = NULL;
    count = 0;
}

ChargingQueue::~ChargingQueue()
{
    while (!isEmpty())
    {
        dequeue();
    }
}

void ChargingQueue::enqueue(string robotID)
{
    QueueEntry* newEntry = new QueueEntry;
    newEntry->robotID = robotID;
    newEntry->next = NULL;

    if (rear == NULL)
    {
        front = newEntry;
        rear = newEntry;
    }
    else
    {
        rear->next = newEntry;
        rear = newEntry;
    }

    count++;
}

string ChargingQueue::dequeue()
{
    if (front == NULL)
    {
        return "";
    }

    QueueEntry* oldFront = front;
    string robotID = oldFront->robotID;
    front = front->next;

    if (front == NULL)
    {
        rear = NULL;
    }

    delete oldFront;
    count--;
    return robotID;
}

bool ChargingQueue::remove(string robotID)
{
    QueueEntry* previous = NULL;
    QueueEntry* current = front;

    while (current != NULL)
    {
        if (current->robotID == robotID)
        {
            if (previous == NULL)
            {
                front = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            if (current == rear)
            {
                rear = previous;
            }

            delete current;
            count--;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

bool ChargingQueue::contains(string robotID)
{
    QueueEntry* current = front;

    while (current != NULL)
    {
        if (current->robotID == robotID)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

string ChargingQueue::getFront()
{
    if (front == NULL)
    {
        return "";
    }

    return front->robotID;
}

bool ChargingQueue::isEmpty()
{
    return front == NULL;
}

int ChargingQueue::getSize()
{
    return count;
}

void ChargingQueue::display()
{
    cout << "Front";
    QueueEntry* current = front;

    while (current != NULL)
    {
        cout << " -> " << current->robotID;
        current = current->next;
    }

    cout << " -> NULL" << endl;
}