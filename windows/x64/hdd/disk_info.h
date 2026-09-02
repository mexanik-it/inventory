#pragma once

#include <string>
#include <vector>

struct DiskInfo {
    std::wstring model;
    std::wstring serialNumber;
    std::wstring type;
    unsigned long long sizeBytes;
};

std::vector<DiskInfo> getDisks();
std::wstring formatDiskSize(unsigned long long bytes);  // <-- добавляем объявление
