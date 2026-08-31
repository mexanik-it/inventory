#ifndef COLLECTOR_STORAGE_H
#define COLLECTOR_STORAGE_H

#include "Types.h"

class CollectorStorage {
public:
    void collect(std::vector<StorageDevice>& target);
};

#endif