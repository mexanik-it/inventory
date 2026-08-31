#include "CollectorStorage.h"
#include <iostream>

void CollectorStorage::collect(std::vector<StorageDevice>& target) {
    std::cout << "[LOG] Storage collector stub called." << std::endl;
    target.push_back({"SSD", "Samsung", "980 PRO", 1000, "S1A2B3C4"});
    target.push_back({"HDD", "Seagate", "Barracuda", 2000, "H5D6E7F8"});
}