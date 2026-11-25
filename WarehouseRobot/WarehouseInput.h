#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Check karta hai ki ID sahi prefix se start hoti hai aur baad mein sirf digits hain
// Example: validWarehouseID("P101", "P") -> true, "RK01" with "RK" -> true
inline bool validWarehouseID(const std::string& id, const std::string& prefix){
    // ID ki length prefix se badi honi chahiye (kam se kam 1 digit hona chahiye)
    if (id.size() <= prefix.size()) {
        return false;
    }

    // Pehle prefix match kar rahe hain (case-sensitive)
    for (size_t i = 0; i < prefix.size(); i++) {
        if (id[i] != prefix[i]) {
            return false;
        }
    }

    // Ab baaki sab characters digits (0-9) hone chahiye
    for (size_t i = prefix.size(); i < id.size(); i++) {
        if (id[i] < '0' || id[i] > '9') {
            return false;
        }
    }

    return true;
}

// Check karta hai ki line mein koi extra ya galat text toh nahi bacha (jaise "4x")
inline bool endOfRecord(std::istream& input){
    input >> std::ws; // spaces skip karo
    return input.eof();
}
