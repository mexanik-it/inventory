#include "CollectorRAM.h"
#include <iostream>

void CollectorRAM::collect(RAMInfo& target) {
    std::cout << "[LOG] RAM collector stub called." << std::endl;
    target.vendor = "Kingston";
    target.modules_count = 2;
    target.total_gb = 32.0;
}