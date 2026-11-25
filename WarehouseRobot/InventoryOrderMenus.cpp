#include "InventoryManager.h"
#include "OrderManager.h"
#include "WarehouseInput.h"
#include <iostream>

using namespace std;

// Inventory management ka interactive console menu
void InventoryManager::showInventoryMenu(){
    cout << "\nInventory commands (one command per line)\n"
         << "1 productID name weightKg reorderLevel   Add product (use underscores in names)\n"
         << "2 productID                             Delete product\n"
         << "3 productID                             Search product and display stock by rack\n"
         << "4 productID rackID quantity             Add rack entry\n"
         << "5 productID rackID newTotal             Adjust physical stock\n"
         << "6 productID rackID                      Remove empty rack entry\n"
         << "7                                       Show products below reorder level\n"
         << "8 productID quantity locationID         Preview rack allocation\n"
         << "0                                       Return\n";

    string command;
    while (true) {
        cout << "Inventory> ";
        if (!getline(cin, command)) {
            break;
        }

        istringstream fields(command);
        int choice = -1;
        if (!(fields >> choice)) {
            cout << "Enter a numbered command.\n";
            continue;
        }

        int quantity = 0;
        double weight = 0.0;
        string productID, rackID, name, locationID;
        bool success = false;

        if (choice == 0) {
            if (endOfRecord(fields)) {
                return;
            }
        } else if (choice == 1) {
            // Naya product add karo
            if ((fields >> productID >> name >> weight >> quantity) && endOfRecord(fields)) {
                success = addProduct(productID, name, weight, quantity);
            }
        } else if (choice == 2) {
            // Product delete karo
            if ((fields >> productID) && endOfRecord(fields)) {
                success = deleteProduct(productID);
            }
        } else if (choice == 3) {
            // Product search karke uske racks dikhao
            if ((fields >> productID) && endOfRecord(fields)) {
                displayRacks(productID);
                continue;
            }
        } else if (choice == 4) {
            // Product ke liye naya rack entry add karo
            if ((fields >> productID >> rackID >> quantity) && endOfRecord(fields)) {
                success = addRackEntry(productID, rackID, quantity);
            }
        } else if (choice == 5) {
            // Existing rack ka physical stock update karo
            if ((fields >> productID >> rackID >> quantity) && endOfRecord(fields)) {
                success = updateQuantity(productID, rackID, quantity);
            }
        } else if (choice == 6) {
            // Empty rack entry remove karo
            if ((fields >> productID >> rackID) && endOfRecord(fields)) {
                success = removeRackEntry(productID, rackID);
            }
        } else if (choice == 7) {
            // Low stock products dikhao
            if (endOfRecord(fields)) {
                displayLowStock();
                continue;
            }
        } else if (choice == 8) {
            // Rack allocation preview karo (sirf plan dikhata hai, reserve nahi karta)
            if ((fields >> productID >> quantity >> locationID) && endOfRecord(fields)) {
                if (quantity > 0 && searchProduct(productID) != nullptr &&
                    warehouse.findLocation(locationID) != nullptr) {
                    RackAllocation proposal = allocateRacks(productID, quantity, locationID);
                    const AllocationLine* line = proposal.lines();
                    while (line != nullptr) {
                        cout << line->rackID << " at " << line->locationID
                             << ": " << line->quantity << " units\n";
                        line = line->next;
                    }
                    cout << "Requested " << quantity
                         << " | reachable " << proposal.allocatedQuantity()
                         << " | shortage " << proposal.shortage() << "\n";
                    continue;
                }
            }
        }

        if (success) {
            cout << "Updated.\n";
        } else {
            cout << "Rejected. Check the command, IDs and stock limits.\n";
        }
    }
}

// Customer order management ka interactive console menu
void OrderManager::showOrderMenu(){
    cout << "\nOrder commands (one command per line)\n"
         << "1 orderID customerID productID quantity   Create order\n"
         << "2 orderID productID quantity              Add or merge an item\n"
         << "3 orderID                                Search order\n"
         << "4                                        Display pending queue\n"
         << "5                                        Peek and show queue size\n"
         << "6 orderID                                Cancel order\n"
         << "7                                        Display completed orders\n"
         << "0                                        Return\n";

    string command;
    while (true) {
        cout << "Orders> ";
        if (!getline(cin, command)) {
            break;
        }

        istringstream fields(command);
        int choice = -1;
        if (!(fields >> choice)) {
            cout << "Enter a numbered command.\n";
            continue;
        }

        int quantity = 0;
        string orderID, customerID, productID;
        bool success = false;

        if (choice == 0) {
            if (endOfRecord(fields)) {
                return;
            }
        } else if (choice == 1) {
            // Naya order banao
            if ((fields >> orderID >> customerID >> productID >> quantity) && endOfRecord(fields)) {
                success = addOrder(orderID, customerID, productID, quantity);
            }
        } else if (choice == 2) {
            // Order mein item add ya merge karo
            if ((fields >> orderID >> productID >> quantity) && endOfRecord(fields)) {
                success = addItem(orderID, productID, quantity);
            }
        } else if (choice == 3) {
            // Order ID se search karo
            if ((fields >> orderID) && endOfRecord(fields)) {
                Order* order = searchOrder(orderID);
                if (order != nullptr) {
                    order->display();
                } else {
                    cout << "Unknown order.\n";
                }
                continue;
            }
        } else if (choice == 4) {
            // Saare pending orders dikhao
            if (endOfRecord(fields)) {
                displayPending();
                continue;
            }
        } else if (choice == 5) {
            // Queue ka front order aur total size dikhao
            if (endOfRecord(fields)) {
                Order* oldest = peekFrontOrder();
                if (oldest != nullptr) {
                    cout << "Front: " << oldest->orderID << " | size " << queueSize() << "\n";
                } else {
                    cout << "Front: (empty) | size " << queueSize() << "\n";
                }
                continue;
            }
        } else if (choice == 6) {
            // Order cancel karo
            if ((fields >> orderID) && endOfRecord(fields)) {
                success = cancelOrder(orderID);
            }
        } else if (choice == 7) {
            // Completed orders dikhao
            if (endOfRecord(fields)) {
                displayCompleted();
                continue;
            }
        }

        if (success) {
            cout << "Updated.\n";
        } else {
            cout << "Rejected. Check the command, IDs and order status.\n";
        }
    }
}
