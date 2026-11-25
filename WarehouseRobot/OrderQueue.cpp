#include "OrderQueue.h"
#include <iostream>

using namespace std;

OrderQueue::OrderQueue(){
    front = nullptr;
    rear = nullptr;
    count = 0;
}

// Queue ke end mein pending order add karo (normal FIFO)
bool OrderQueue::enqueue(Order* order){
    if (order == nullptr || order->queued || order->status != PENDING) {
        return false;
    }

    order->nextPending = nullptr;
    if (rear != nullptr) {
        rear->nextPending = order;
    } else {
        front = order;
    }
    rear = order;
    order->queued = true;
    count++;
    return true;
}

// Replanning ke time order ko queue ke bilkul front mein lagane ke liye
bool OrderQueue::prepend(Order* order){
    if (order == nullptr || order->queued || order->status != PENDING) {
        return false;
    }

    order->nextPending = front;
    front = order;
    if (rear == nullptr) {
        rear = order;
    }
    order->queued = true;
    count++;
    return true;
}

// Sabse purana pending order queue se nikalo
Order* OrderQueue::dequeue(){
    Order* oldest = front;
    if (oldest != nullptr) {
        remove(oldest);
    }
    return oldest;
}

// Queue se koi bhi specific order remove karo (front, middle ya end)
bool OrderQueue::remove(Order* order){
    if (order == nullptr || !order->queued) {
        return false;
    }

    Order* prev = nullptr;
    Order* curr = front;

    while (curr != nullptr && curr != order) {
        prev = curr;
        curr = curr->nextPending;
    }

    if (curr == nullptr) {
        return false;
    }

    // Node ko queue se unlink karo
    if (prev != nullptr) {
        prev->nextPending = curr->nextPending;
    } else {
        front = curr->nextPending;
    }

    if (rear == curr) {
        rear = prev;
    }

    curr->nextPending = nullptr;
    curr->queued = false;
    count--;
    return true;
}

// Saare pending orders FIFO order mein display karo
void OrderQueue::display() const{
    if (front == nullptr) {
        cout << "Pending queue is empty.\n";
        return;
    }

    Order* curr = front;
    while (curr != nullptr) {
        curr->display();
        curr = curr->nextPending;
    }
}
