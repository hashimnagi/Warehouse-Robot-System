#include "Product.h"

using namespace std;

// Constructor: product ki basic info set karo
Product::Product(const string& identifier, const string& productName, double weight, int threshold){
    productID = identifier;
    name = productName;
    unitWeight = weight;
    reorderLevel = threshold;
    racks = nullptr;
}

// Destructor: product delete hone par uske saare racks bhi delete kar do
Product::~Product(){
    RackEntry* curr = racks;
    while (curr != nullptr) {
        RackEntry* temp = curr;
        curr = curr->next;
        delete temp;
    }
    racks = nullptr;
}

// Rack ID ke basis par rack entry dhundo
RackEntry* Product::findRack(const string& rackID) const{
    RackEntry* curr = racks;
    while (curr != nullptr) {
        if (curr->rackID == rackID) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

// Saare racks ka total physical stock add karke return karo
long long Product::totalStock() const{
    long long total = 0;
    RackEntry* curr = racks;
    while (curr != nullptr) {
        total += curr->totalQuantity;
        curr = curr->next;
    }
    return total;
}

// Saare racks ka available (unreserved) stock calculate karo
long long Product::availableStock() const{
    long long available = 0;
    RackEntry* curr = racks;
    while (curr != nullptr) {
        available += curr->availableQuantity();
        curr = curr->next;
    }
    return available;
}

// Check karo ki kisi bhi rack par koi active reservation hai ya nahi
bool Product::hasReservations() const{
    RackEntry* curr = racks;
    while (curr != nullptr) {
        if (curr->reservedQuantity > 0) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}
