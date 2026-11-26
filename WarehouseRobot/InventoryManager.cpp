#include "InventoryManager.h"
#include "RackDistances.h"
#include "WarehouseInput.h"
#include <iostream>
#include <climits>

using namespace std;

// ================= RackAllocation Methods =================

RackAllocation::RackAllocation(){
    first = nullptr;
    last = nullptr;
    requested = 0;
    allocated = 0;
    partialAllowed = false;
}

RackAllocation::~RackAllocation(){
    AllocationLine* curr = first;
    while (curr != nullptr) {
        AllocationLine* temp = curr;
        curr = curr->next;
        delete temp;
    }
    first = nullptr;
    last = nullptr;
}

// Move constructor: proposal return karte waqt linked list ke pointers shift kar dete hain
RackAllocation::RackAllocation(RackAllocation&& source) noexcept {
    first = source.first;
    last = source.last;
    requested = source.requested;
    allocated = source.allocated;
    partialAllowed = source.partialAllowed;

    source.first = nullptr;
    source.last = nullptr;
    source.requested = 0;
    source.allocated = 0;
    source.partialAllowed = false;
}

// Proposal list ke end mein ek nayi allocation line add karo
void RackAllocation::append(const string& productID, const string& rackID,
                            const string& locationID, int quantity){
    AllocationLine* line = new AllocationLine();
    line->productID = productID;
    line->rackID = rackID;
    line->locationID = locationID;
    line->quantity = quantity;
    line->next = nullptr;

    if (last != nullptr) {
        last->next = line;
    } else {
        first = line;
    }
    last = line;
    allocated += quantity;
}

// ================= InventoryManager Methods =================

InventoryManager::InventoryManager(Graph& warehouseGraph) : warehouse(warehouseGraph){
    mappings = nullptr;
    distanceLookup = nullptr;
    recordChange = nullptr;
}

InventoryManager::~InventoryManager(){
    RackMapping* curr = mappings;
    while (curr != nullptr) {
        RackMapping* temp = curr;
        curr = curr->next;
        delete temp;
    }
    mappings = nullptr;
}

// Rack ID ke corresponding Location mapping dhundo
InventoryManager::RackMapping* InventoryManager::findMapping(const string& rackID) const{
    RackMapping* curr = mappings;
    while (curr != nullptr) {
        if (curr->rackID == rackID) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

// racks.txt file se saare racks load karo (Person A ke graph mein location RACK honi chahiye)
bool InventoryManager::loadRacks(const string& fileName){
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

        // Blank lines ignore karo
        if (endOfRecord(fields)) {
            continue;
        }

        string rackID, locationID;
        bool rowOk = true;

        if (!(fields >> rackID >> locationID) || !endOfRecord(fields)) {
            rowOk = false;
        } else if (!validWarehouseID(rackID, "RK")) {
            rowOk = false;
        } else if (findMapping(rackID) != nullptr) {
            rowOk = false; // duplicate rack ID
        } else {
            Location* loc = warehouse.findLocation(locationID);
            if (loc == nullptr || loc->getType() != RACK) {
                rowOk = false;
            } else {
                RackMapping* newMap = new RackMapping();
                newMap->rackID = rackID;
                newMap->location = loc;
                newMap->next = mappings;
                mappings = newMap;
            }
        }

        if (!rowOk) {
            cerr << fileName << ", line " << lineNumber << " rejected.\n";
            allValid = false;
        }
    }

    return allValid && !file.bad();
}

// products.txt se saare products AVL tree mein load karo
bool InventoryManager::loadProducts(const string& fileName){
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

        string productID, name;
        double weight = 0.0;
        int threshold = 0;
        bool rowOk = false;

        if ((fields >> productID >> name >> weight >> threshold) && endOfRecord(fields)) {
            rowOk = addProduct(productID, name, weight, threshold);
        }

        if (!rowOk) {
            cerr << fileName << ", line " << lineNumber << " rejected.\n";
            allValid = false;
        }
    }

    return allValid && !file.bad();
}

// inventory.txt se product-rack stock entries load karo
bool InventoryManager::loadInventory(const string& fileName){
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

        string productID, rackID;
        int quantity = 0;
        bool rowOk = false;

        if ((fields >> productID >> rackID >> quantity) && endOfRecord(fields)) {
            rowOk = addRackEntry(productID, rackID, quantity);
        }

        if (!rowOk) {
            cerr << fileName << ", line " << lineNumber << " rejected.\n";
            allValid = false;
        }
    }

    return allValid && !file.bad();
}

bool InventoryManager::addProduct(const string& productID, const string& name,
                                  double weight, int reorderLevel){
    return products.insert(productID, name, weight, reorderLevel);
}

bool InventoryManager::deleteProduct(const string& productID){
    return products.erase(productID);
}

Product* InventoryManager::searchProduct(const string& productID) const{
    return products.search(productID);
}

// Product ke andar specific rack entry dhundne ka helper
RackEntry* InventoryManager::lookupEntry(const string& productID, const string& rackID) const{
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return nullptr;
    }
    return product->findRack(rackID);
}

// Agar product ya rack nahi mila toh -1 return hota hai (0 valid stock ho sakta hai)
double InventoryManager::getUnitWeight(const string& productID) const{
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return -1;
    }
    return product->unitWeight;
}

long long InventoryManager::getTotalStock(const string& productID) const{
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return -1;
    }
    return product->totalStock();
}

long long InventoryManager::getAvailableStock(const string& productID) const{
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return -1;
    }
    return product->availableStock();
}

int InventoryManager::getReservedQty(const string& productID, const string& rackID) const{
    RackEntry* rack = lookupEntry(productID, rackID);
    if (rack == nullptr) {
        return -1;
    }
    return rack->reservedQuantity;
}

// Product ke liye naya rack entry add karo
bool InventoryManager::addRackEntry(const string& productID, const string& rackID, int quantity){
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return false;
    }
    if (findMapping(rackID) == nullptr) {
        return false; // rack exist nahi karta
    }
    if (quantity < 0) {
        return false;
    }
    if (product->findRack(rackID) != nullptr) {
        return false; // is rack par entry pehle se hai
    }

    RackEntry* rack = new RackEntry(rackID, quantity);
    rack->next = product->racks;
    product->racks = rack;
    return true;
}

// Sirf empty aur unreserved rack entry hi remove ho sakti hai
bool InventoryManager::removeRackEntry(const string& productID, const string& rackID){
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        return false;
    }

    RackEntry* prev = nullptr;
    RackEntry* curr = product->racks;

    while (curr != nullptr && curr->rackID != rackID) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == nullptr) {
        return false;
    }
    if (curr->totalQuantity != 0 || curr->reservedQuantity != 0) {
        return false;
    }

    // Linked list se node unlink karo
    if (prev == nullptr) {
        product->racks = curr->next;
    } else {
        prev->next = curr->next;
    }
    delete curr;
    return true;
}

// Undo ke liye purani quantity wapas set karo (history callback trigger nahi hoga)
bool InventoryManager::revertQuantityUpdate(const string& productID, const string& rackID, int oldTotal){
    RackEntry* rack = lookupEntry(productID, rackID);
    if (rack == nullptr || oldTotal < rack->reservedQuantity) {
        return false;
    }
    rack->totalQuantity = oldTotal;
    return true;
}

// Manual stock update karo; agar undo callback set hai toh usko notify karo
bool InventoryManager::updateQuantity(const string& productID, const string& rackID, int newTotal){
    RackEntry* rack = lookupEntry(productID, rackID);
    if (rack == nullptr || newTotal < rack->reservedQuantity) {
        return false;
    }

    int oldTotal = rack->totalQuantity;
    rack->totalQuantity = newTotal;

    if (recordChange != nullptr && oldTotal != newTotal) {
        recordChange(productID, rackID, oldTotal, newTotal);
    }
    return true;
}

void InventoryManager::displayRacks(const string& productID) const{
    Product* product = searchProduct(productID);
    if (product == nullptr) {
        cout << "Unknown product.\n";
        return;
    }

    cout << productID << " " << product->name << " | " << product->unitWeight
         << " kg | reorder " << product->reorderLevel << "\n";
    cout << "Rack  Total  Reserved  Available\n";

    RackEntry* rack = product->racks;
    while (rack != nullptr) {
        cout << rack->rackID << "  " << rack->totalQuantity << "  "
             << rack->reservedQuantity << "  " << rack->availableQuantity() << "\n";
        rack = rack->next;
    }

    cout << "Total stock: " << product->totalStock()
         << " | Available: " << product->availableStock() << "\n";
}

void InventoryManager::displayLowStock() const{
    products.displayLowStock();
}

// Best racks choose karne ka simple logic:
// 1. Pehle woh rack prefer karo jo poori remaining quantity akela de sake
// 2. Phir sabse kam distance wala rack
// 3. Phir zyada available stock wala rack
// 4. Tie hone par chhota rackID (alphabetically)
RackAllocation InventoryManager::allocateRacks(const string& productID, int quantity,
                                               const string& referenceLocation, bool allowPartial) const{
    RackAllocation proposal;
    if (quantity <= 0) {
        return proposal;
    }

    proposal.requested = quantity;
    proposal.partialAllowed = allowPartial;

    Product* product = searchProduct(productID);
    if (product == nullptr || warehouse.findLocation(referenceLocation) == nullptr) {
        return proposal;
    }

    // Agar team ka distance callback nahi hai toh apna RackDistances (Dijkstra) run karo
    string refForLocal = "";
    if (distanceLookup == nullptr) {
        refForLocal = referenceLocation;
    }
    RackDistances distances(warehouse, refForLocal);

    // Jab tak required quantity poori na ho jaye, ek-ek karke best rack pick karo
    while (proposal.shortage() > 0) {
        RackEntry* bestRack = nullptr;
        RackMapping* bestMapping = nullptr;
        long long bestDistance = -1;
        bool bestFills = false;

        RackEntry* rack = product->racks;
        while (rack != nullptr) {
            int available = rack->availableQuantity();

            // Check karo ki rack mein stock hai aur pehle se proposal mein add toh nahi hua
            bool alreadyUsed = false;
            const AllocationLine* used = proposal.lines();
            while (used != nullptr) {
                if (used->rackID == rack->rackID) {
                    alreadyUsed = true;
                    break;
                }
                used = used->next;
            }

            if (available > 0 && !alreadyUsed) {
                RackMapping* mapping = findMapping(rack->rackID);
                if (mapping != nullptr) {
                    long long dist = -1;
                    if (distanceLookup != nullptr) {
                        dist = distanceLookup(referenceLocation.c_str(), mapping->location->getID().c_str());
                        if (dist == INT_MAX) {
                            dist = -1;
                        }
                    } else {
                        dist = distances.distanceTo(mapping->location);
                    }

                    // Sirf reachable racks consider karo
                    if (dist >= 0) {
                        bool fills = (available >= proposal.shortage());
                        bool isBetter = false;

                        if (bestRack == nullptr) {
                            isBetter = true;
                        } else if (fills && !bestFills) {
                            isBetter = true;
                        } else if (fills == bestFills) {
                            if (dist < bestDistance) {
                                isBetter = true;
                            } else if (dist == bestDistance && available > bestRack->availableQuantity()) {
                                isBetter = true;
                            } else if (dist == bestDistance && available == bestRack->availableQuantity() &&
                                       rack->rackID < bestRack->rackID) {
                                isBetter = true;
                            }
                        }

                        if (isBetter) {
                            bestRack = rack;
                            bestMapping = mapping;
                            bestDistance = dist;
                            bestFills = fills;
                        }
                    }
                }
            }

            rack = rack->next;
        }

        // Agar koi bhi valid rack nahi mila toh loop rok do
        if (bestRack == nullptr) {
            break;
        }

        int taken = 0;
        if (bestFills) {
            taken = proposal.shortage();
        } else {
            taken = bestRack->availableQuantity();
        }

        proposal.append(productID, bestRack->rackID, bestMapping->location->getID(), taken);
    }

    return proposal;
}

bool InventoryManager::reserveStock(const RackAllocation* allocation, const string& orderID){
    if (allocation == nullptr) {
        return false;
    }
    PlanPart singlePart{*allocation, nullptr};
    return reserveStock(singlePart, orderID);
}

// Multi-product plan ko pehle poora verify karo, phir sab ek saath reserve karo
bool InventoryManager::reserveStock(const PlanPart& plan, const string& orderID){
    if (!validWarehouseID(orderID, "O")) {
        return false;
    }

    // Step 1: Har part aur har line check karo ki stock enough hai ya nahi
    const PlanPart* part = &plan;
    while (part != nullptr) {
        if (!part->allocation.reservable()) {
            return false;
        }

        const AllocationLine* line = part->allocation.lines();
        while (line != nullptr) {
            RackEntry* rack = lookupEntry(line->productID, line->rackID);
            if (rack == nullptr) {
                return false;
            }

            // Agar plan mein same product+rack do baar aaya ho toh total demand check karo
            long long totalDemand = 0;
            const PlanPart* otherPart = &plan;
            while (otherPart != nullptr) {
                const AllocationLine* otherLine = otherPart->allocation.lines();
                while (otherLine != nullptr) {
                    if (otherLine->productID == line->productID && otherLine->rackID == line->rackID) {
                        totalDemand += otherLine->quantity;
                    }
                    otherLine = otherLine->next;
                }
                otherPart = otherPart->next;
            }

            if (totalDemand > rack->availableQuantity()) {
                return false;
            }

            line = line->next;
        }
        part = part->next;
    }

    // Step 2: Sab valid hai, ab actual reservation apply kar do
    part = &plan;
    while (part != nullptr) {
        const AllocationLine* line = part->allocation.lines();
        while (line != nullptr) {
            RackEntry* rack = lookupEntry(line->productID, line->rackID);
            rack->reserve(orderID, line->quantity);
            line = line->next;
        }
        part = part->next;
    }

    return true;
}

bool InventoryManager::releaseReservation(const string& orderID, const string& productID,
                                          const string& rackID, int quantity){
    RackEntry* rack = lookupEntry(productID, rackID);
    if (rack == nullptr) {
        return false;
    }
    return rack->release(orderID, quantity, false);
}

// Robot jab rack se item utha le tab physical stock aur reservation dono kam karo
bool InventoryManager::deductStock(const string& productID, const string& rackID,
                                   int quantity, const string& orderID){
    RackEntry* rack = lookupEntry(productID, rackID);
    if (rack == nullptr) {
        return false;
    }
    return rack->release(orderID, quantity, true);
}

// Order cancel ya requeue hone par uske saare reservations free karo
void InventoryManager::releaseOrderReservations(const string& orderID){
    products.releaseOrderReservations(orderID);
}
