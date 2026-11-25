#include "RackEntry.h"

using namespace std;

// Constructor: rack ki initial details set karo
RackEntry::RackEntry(const string& identifier, int quantity){
    rackID = identifier;
    totalQuantity = quantity;
    reservedQuantity = 0;
    reservations = nullptr;
    next = nullptr;
}

// Destructor: saare reservation nodes delete kar do taaki memory leak na ho
RackEntry::~RackEntry(){
    ReservationRecord* curr = reservations;
    while (curr != nullptr) {
        ReservationRecord* temp = curr;
        curr = curr->next;
        delete temp;
    }
    reservations = nullptr;
}

// Kitna stock abhi free hai (total minus reserved)
int RackEntry::availableQuantity() const{
    return totalQuantity - reservedQuantity;
}

// Kisi order ke liye stock reserve karna
// Agar same order ne pehle bhi is rack pe reserve kiya hai toh usi mein add kar do
bool RackEntry::reserve(const string& orderID, int quantity){
    if (quantity <= 0) {
        return false;
    }
    if (quantity > availableQuantity()) {
        return false;
    }

    // Check karo ki is order ka record pehle se hai kya
    ReservationRecord* curr = reservations;
    while (curr != nullptr) {
        if (curr->orderID == orderID) {
            curr->quantity += quantity;
            reservedQuantity += quantity;
            return true;
        }
        curr = curr->next;
    }

    // Agar record nahi mila toh naya node bana ke start mein laga do
    ReservationRecord* newRec = new ReservationRecord();
    newRec->orderID = orderID;
    newRec->quantity = quantity;
    newRec->next = reservations;
    reservations = newRec;

    reservedQuantity += quantity;
    return true;
}

// Reservation ko release karna; agar collected == true hai toh physical stock bhi kam hoga
bool RackEntry::release(const string& orderID, int quantity, bool collected){
    if (quantity <= 0) {
        return false;
    }

    ReservationRecord* prev = nullptr;
    ReservationRecord* curr = reservations;

    // Order ka reservation node dhundo
    while (curr != nullptr && curr->orderID != orderID) {
        prev = curr;
        curr = curr->next;
    }

    // Agar record nahi mila ya zyada quantity release karne ki koshish ho rahi hai
    if (curr == nullptr || quantity > curr->quantity) {
        return false;
    }

    curr->quantity -= quantity;
    reservedQuantity -= quantity;

    // Jab robot item pick kar leta hai tab total physical stock bhi ghat jaata hai
    if (collected) {
        totalQuantity -= quantity;
    }

    // Agar is order ki reserved quantity 0 ho gayi toh node ko list se hata do
    if (curr->quantity == 0) {
        if (prev == nullptr) {
            reservations = curr->next;
        } else {
            prev->next = curr->next;
        }
        delete curr;
    }

    return true;
}

// Kisi order ke saare reservations is rack se hata do
void RackEntry::releaseAll(const string& orderID){
    ReservationRecord* curr = reservations;
    while (curr != nullptr) {
        if (curr->orderID == orderID) {
            release(orderID, curr->quantity, false);
            return;
        }
        curr = curr->next;
    }
}
