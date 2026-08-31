#include "CollectorMB.h"
#include <iostream>

void CollectorMB::collect(MotherboardInfo& target) {
    std::cout << "[LOG] MB collector stub called." << std::endl;
    target.vendor = "ASUSTeK COMPUTER INC.";
    target.model = "ROG MAXIMUS Z790 HERO";
    target.serial = "M1234567890";
}