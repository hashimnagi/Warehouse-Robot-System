#include <iostream>
#include "Graph.h"
#include "BFS.h"

LocationQueue::LocationQueue()
{
    front = NULL;
    rear = NULL;
}

LocationQueue::~LocationQueue()
{
    while (!isEmpty())
    {
        dequeue();
    }
}

void LocationQueue::enqueue(Location* location)
{
    QueueNode* newNode = new QueueNode;
    newNode->location = location;
    newNode->next = NULL;

    if (rear == NULL)
    {
        front = newNode;
        rear = newNode;
        return;
    }

    rear->next = newNode;
    rear = newNode;
}

Location* LocationQueue::dequeue()
{
    if (front == NULL)
    {
        return NULL;
    }

    QueueNode* oldFront = front;
    Location* location = oldFront->location;
    front = front->next;

    if (front == NULL)
    {
        rear = NULL;
    }

    delete oldFront;
    return location;
}

bool LocationQueue::isEmpty()
{
    return front == NULL;
}

bool Graph::isConnected(string fromID, string toID)
{
    Location* start = findLocation(fromID);
    Location* end = findLocation(toID);

    if (start == NULL)
    {
        cout << "Location " << fromID << " does not exist." << endl;
        return false;
    }

    if (end == NULL)
    {
        cout << "Location " << toID << " does not exist." << endl;
        return false;
    }

    resetSearchData();
    LocationQueue waitingQueue;
    start->searchVisited = true;
    waitingQueue.enqueue(start);

    while (!waitingQueue.isEmpty())
    {
        Location* current = waitingQueue.dequeue();

        if (current == end)
        {
            return true;
        }

        Edge* edge = current->getFirstEdge();

        while (edge != NULL)
        {
            Location* neighbor = edge->destination;

            if (edge->status == OPEN && !neighbor->searchVisited)
            {
                neighbor->searchVisited = true;
                waitingQueue.enqueue(neighbor);
            }

            edge = edge->next;
        }
    }

    return false;
}