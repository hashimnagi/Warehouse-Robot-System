#include "RackDistances.h"
#include <climits>

using namespace std;

// Diye gaye location ka DistanceEntry node list mein dhundo
RackDistances::DistanceEntry* RackDistances::find(Location* location) const{
    DistanceEntry* curr = entries;
    while (curr != nullptr) {
        if (curr->location == location) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

// Simple Dijkstra algorithm: referenceLocation se saare reachable locations tak ka distance nikalo
RackDistances::RackDistances(Graph& warehouse, const string& referenceLocation){
    entries = nullptr;

    Location* start = warehouse.findLocation(referenceLocation);
    if (start == nullptr) {
        return;
    }

    // Step 1: Graph ke saare locations ke liye entry banao
    // Start location ka distance 0 aur baaki sab ka LLONG_MAX (infinity)
    Location* loc = warehouse.getFirstLocation();
    while (loc != nullptr) {
        DistanceEntry* node = new DistanceEntry();
        node->location = loc;
        if (loc == start) {
            node->distance = 0;
        } else {
            node->distance = LLONG_MAX;
        }
        node->visited = false;
        node->next = entries;
        entries = node;

        loc = loc->getNext();
    }

    // Step 2: Har step mein sabse kam distance wala unvisited location pick karo
    while (true) {
        DistanceEntry* nearest = nullptr;
        DistanceEntry* curr = entries;
        while (curr != nullptr) {
            if (!curr->visited && curr->distance != LLONG_MAX) {
                if (nearest == nullptr || curr->distance < nearest->distance) {
                    nearest = curr;
                }
            }
            curr = curr->next;
        }

        // Agar koi reachable unvisited node nahi bacha toh loop khatam
        if (nearest == nullptr) {
            break;
        }

        nearest->visited = true;

        // Step 3: Nearest node ke saare OPEN edges check karo aur neighbours ka distance update karo
        Edge* edge = nearest->location->getFirstEdge();
        while (edge != nullptr) {
            if (edge->status == OPEN && edge->distance > 0) {
                DistanceEntry* neighbour = find(edge->destination);
                if (neighbour != nullptr && !neighbour->visited) {
                    // Overflow se bachne ke liye check
                    if (nearest->distance <= LLONG_MAX - edge->distance) {
                        long long newDist = nearest->distance + edge->distance;
                        if (newDist < neighbour->distance) {
                            neighbour->distance = newDist;
                        }
                    }
                }
            }
            edge = edge->next;
        }
    }
}

// Kisi location tak ka shortest distance return karo; agar reachable nahi hai toh -1
long long RackDistances::distanceTo(Location* location) const{
    DistanceEntry* entry = find(location);
    if (entry == nullptr || entry->distance == LLONG_MAX) {
        return -1;
    }
    return entry->distance;
}

// Destructor: saare DistanceEntry nodes free kar do
RackDistances::~RackDistances(){
    DistanceEntry* curr = entries;
    while (curr != nullptr) {
        DistanceEntry* temp = curr;
        curr = curr->next;
        delete temp;
    }
    entries = nullptr;
}
