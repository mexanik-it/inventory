// в main.cpp
#include <iostream>
#include "disk_info.h"

int main() {
    auto disks = getDisks();
    for (const auto& d : disks) {
        std::wcout << L"Model: " << d.model
                   << L"\n  Serial: " << d.serialNumber
                   << L"\n  Type: " << d.type
                   << L"\n  Size: " << formatDiskSize(d.sizeBytes)
                   << L" (" << d.sizeBytes << L" bytes)\n\n";
    }
    return 0;
}
