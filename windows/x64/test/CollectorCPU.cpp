#include "CollectorCPU.h"
#include <iostream>

void CollectorCPU::collect(CPUInfo& target) {
    std::cout << "[LOG] CPU collector stub called." << std::endl;
    target.vendor = "Intel";
    target.model = "Core i7-14700K";
    target.serial = "BFEBFBFF000B06F1";
}