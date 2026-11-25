#include "Order.h"
#include <climits>
#include <iostream>

using namespace std;

// Constructor: naya order hamesha PENDING state mein start hota hai
Order::Order(const string& identifier, const string& customer)
    : orderID(identifier), customerID(customer){
    nextOrder = nullptr;
    nextPending = nullptr;
    queued = false;
    tripStarted = false;
    status = PENDING;
    firstItem = nullptr;
    lastItem = nullptr;
}

// Destructor: order ke saare items delete karo
Order::~Order(){
    OrderItem* curr = firstItem;
    while (curr != nullptr) {
        OrderItem* temp = curr;
        curr = curr->next;
        delete temp;
    }
    firstItem = nullptr;
    lastItem = nullptr;
}

// Order ke andar productID dhundo
OrderItem* Order::findItem(const string& productID) const{
    OrderItem* curr = firstItem;
    while (curr != nullptr) {
        if (curr->productID == productID) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

// Order mein item add karo; agar same product dobara aaye toh quantity merge kar do
bool Order::appendItem(const string& productID, int quantity){
    if (quantity <= 0 || status != PENDING) {
        return false;
    }

    OrderItem* existing = findItem(productID);
    if (existing != nullptr) {
        // Integer overflow se bachne ke liye check
        if (quantity > INT_MAX - existing->requiredQuantity) {
            return false;
        }
        existing->requiredQuantity += quantity;
        return true;
    }

    OrderItem* newItem = new OrderItem();
    newItem->productID = productID;
    newItem->requiredQuantity = quantity;
    newItem->deliveredQuantity = 0;
    newItem->next = nullptr;

    if (lastItem != nullptr) {
        lastItem->next = newItem;
    } else {
        firstItem = newItem;
    }
    lastItem = newItem;
    return true;
}

// Check karo ki koi bhi item thoda bahut deliver hua hai ya nahi
bool Order::hasDelivery() const{
    OrderItem* curr = firstItem;
    while (curr != nullptr) {
        if (curr->deliveredQuantity > 0) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

// Check karo ki saare items 100% deliver ho chuke hain ya nahi
bool Order::fullyDelivered() const{
    if (firstItem == nullptr) {
        return false;
    }
    OrderItem* curr = firstItem;
    while (curr != nullptr) {
        if (curr->deliveredQuantity != curr->requiredQuantity) {
            return false;
        }
        curr = curr->next;
    }
    return true;
}

// Enum status ko readable text mein convert karo
static const char* statusName(OrderStatus status){
    if (status == PENDING) return "Pending";
    if (status == RESERVED) return "Reserved";
    if (status == ASSIGNED_ORDER) return "Assigned";
    if (status == PARTIALLY_FULFILLED) return "Partially Fulfilled";
    if (status == COMPLETED) return "Completed";
    if (status == CANCELLED) return "Cancelled";
    return "Unknown";
}

// Order aur uske saare items console pe print karo
void Order::display() const{
    cout << orderID << " | " << customerID << " | " << statusName(status) << "\n";
    OrderItem* curr = firstItem;
    while (curr != nullptr) {
        cout << "  " << curr->productID
             << " | required " << curr->requiredQuantity
             << " | delivered " << curr->deliveredQuantity << "\n";
        curr = curr->next;
    }
}
