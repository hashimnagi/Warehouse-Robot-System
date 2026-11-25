#include "OrderManager.h"
#include "InventoryManager.h"
#include "WarehouseInput.h"
#include <cctype>

using namespace std;

OrderManager::OrderManager(InventoryManager& inventoryManager) : inventory(inventoryManager){
    releaseRobot = nullptr;
}

Order* OrderManager::searchOrder(const string& orderID) const{
    return orders.find(orderID);
}

Order* OrderManager::peekFrontOrder() const{
    return pendingOrders.peek();
}

Order* OrderManager::dequeueOrder(){
    return pendingOrders.dequeue();
}

int OrderManager::queueSize() const{
    return pendingOrders.size();
}

// Naya order create karo aur master list + pending queue dono mein add karo
bool OrderManager::addOrder(const string& orderID, const string& customerID,
                            const string& firstProductID, int quantity){
    if (!validWarehouseID(orderID, "O")) {
        return false;
    }
    if (!validWarehouseID(customerID, "CU")) {
        return false;
    }
    if (searchOrder(orderID) != nullptr) {
        return false; // duplicate order ID
    }
    if (inventory.searchProduct(firstProductID) == nullptr) {
        return false; // product exist nahi karta
    }
    if (quantity <= 0) {
        return false;
    }

    Order* order = new Order(orderID, customerID);
    order->appendItem(firstProductID, quantity);
    orders.append(order);
    return pendingOrders.enqueue(order);
}

// Existing pending order mein aur product add ya merge karo
bool OrderManager::addItem(const string& orderID, const string& productID, int quantity){
    Order* order = searchOrder(orderID);
    if (order == nullptr) {
        return false;
    }
    if (inventory.searchProduct(productID) == nullptr) {
        return false;
    }
    return order->appendItem(productID, quantity);
}

// Ek line se poora order parse karo; agar beech mein koi item galat ho toh poori row reject kar do
bool OrderManager::addRecord(istream& fields){
    string orderID, customerID;
    if (!(fields >> orderID >> customerID)) {
        return false;
    }
    if (!validWarehouseID(orderID, "O") || !validWarehouseID(customerID, "CU")) {
        return false;
    }
    if (searchOrder(orderID) != nullptr) {
        return false;
    }

    Order* order = new Order(orderID, customerID);

    // Line ke end tak saare (productID, quantity) pairs read karo
    while (!endOfRecord(fields)) {
        string productID;
        int quantity = 0;

        if (!(fields >> productID >> quantity)) {
            delete order;
            return false;
        }
        // Check karo ki number ke baad koi galat character (jaise "5x") toh nahi laga
        if (!fields.eof() && !isspace(static_cast<unsigned char>(fields.peek()))) {
            delete order;
            return false;
        }
        if (inventory.searchProduct(productID) == nullptr) {
            delete order;
            return false;
        }
        if (!order->appendItem(productID, quantity)) {
            delete order;
            return false;
        }
    }

    // Kam se kam 1 item hona zaroori hai
    if (order->items() == nullptr) {
        delete order;
        return false;
    }

    orders.append(order);
    return pendingOrders.enqueue(order);
}

// orders.txt file se saare orders load karo
bool OrderManager::loadOrders(const string& fileName){
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Could not open " << fileName << "\n";
        return false;
    }

    string line;
    int lineNumber = 0;
    bool allValid = true;

    while (getline(file, line)) {
        lineNumber++;
        istringstream fields(line);

        if (endOfRecord(fields)) {
            continue;
        }

        if (!addRecord(fields)) {
            cerr << fileName << ", line " << lineNumber << " rejected.\n";
            allValid = false;
        }
    }

    return allValid && !file.bad();
}

// Pehle fleet (robot) ko release karne bolo; agar robot release fail ho toh reservations safe rakho
bool OrderManager::releaseAssignment(Order& order){
    if (order.status == ASSIGNED_ORDER || order.status == PARTIALLY_FULFILLED) {
        if (releaseRobot == nullptr || !releaseRobot(order.orderID)) {
            return false;
        }
    } else if (order.status == RESERVED) {
        if (releaseRobot != nullptr && !releaseRobot(order.orderID)) {
            return false;
        }
    }

    inventory.releaseOrderReservations(order.orderID);
    return true;
}

// Order cancel karo (sirf tab jab trip start na hui ho aur koi delivery na hui ho)
bool OrderManager::cancelOrder(const string& orderID){
    Order* order = searchOrder(orderID);
    if (order == nullptr) {
        return false;
    }
    if (order->tripStarted || order->hasDelivery()) {
        return false;
    }
    if (order->status != PENDING && order->status != RESERVED && order->status != ASSIGNED_ORDER) {
        return false;
    }

    if (!releaseAssignment(*order)) {
        return false;
    }

    pendingOrders.remove(order);
    order->status = CANCELLED;
    return true;
}

// Order ko replanning ke liye pending queue ke front mein wapas daal do
// Jo items deliver ho chuke hain woh safe rehte hain
bool OrderManager::requeueAtFront(Order* order){
    if (order == nullptr) {
        return false;
    }
    if (searchOrder(order->orderID) != order) {
        return false;
    }
    if (order->status == COMPLETED || order->status == CANCELLED) {
        return false;
    }

    if (!releaseAssignment(*order)) {
        return false;
    }

    pendingOrders.remove(order);
    order->status = PENDING;
    return pendingOrders.prepend(order);
}

// Order status transition rules check karke status update karo
bool OrderManager::updateOrderStatus(const string& orderID, OrderStatus newStatus){
    Order* order = searchOrder(orderID);
    if (order == nullptr) {
        return false;
    }
    if (newStatus == order->status) {
        return true;
    }
    if (newStatus == CANCELLED) {
        return cancelOrder(orderID);
    }
    if (newStatus == PENDING) {
        return requeueAtFront(order);
    }

    // PENDING -> RESERVED hone par pending queue se nikal do
    if (newStatus == RESERVED && order->status == PENDING) {
        pendingOrders.remove(order);
        order->status = RESERVED;
        return true;
    }

    // RESERVED -> ASSIGNED_ORDER
    if (newStatus == ASSIGNED_ORDER && order->status == RESERVED) {
        order->status = ASSIGNED_ORDER;
        return true;
    }

    // COMPLETED / PARTIALLY_FULFILLED sirf updateDeliveredQty() se set hote hain
    return false;
}

// Jab robot order pick karne nikal pade tab tripStarted true mark karo
bool OrderManager::markTripStarted(const string& orderID){
    Order* order = searchOrder(orderID);
    if (order == nullptr) {
        return false;
    }
    if (order->status != ASSIGNED_ORDER && order->status != PARTIALLY_FULFILLED) {
        return false;
    }

    order->tripStarted = true;
    return true;
}

// Packing station par delivery aane par delivered quantity badhao aur status update karo
bool OrderManager::updateDeliveredQty(const string& orderID, const string& productID, int quantity){
    Order* order = searchOrder(orderID);
    if (order == nullptr) {
        return false;
    }
    if (order->status != ASSIGNED_ORDER && order->status != PARTIALLY_FULFILLED) {
        return false;
    }

    OrderItem* item = order->findItem(productID);
    if (item == nullptr) {
        return false;
    }

    int remaining = item->requiredQuantity - item->deliveredQuantity;
    if (quantity <= 0 || quantity > remaining) {
        return false;
    }

    item->deliveredQuantity += quantity;
    order->tripStarted = true;

    if (order->fullyDelivered()) {
        order->status = COMPLETED;
    } else {
        order->status = PARTIALLY_FULFILLED;
    }
    return true;
}

void OrderManager::displayPending() const{
    pendingOrders.display();
}

void OrderManager::displayCompleted() const{
    orders.displayStatus(COMPLETED);
}