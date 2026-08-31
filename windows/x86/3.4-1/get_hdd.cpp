#include "main.h"
#include <windows.h>
#include <winioctl.h>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>

static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    if (start == str.size()) return "";

    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        --end;
    }
    return str.substr(start, end - start + 1);
}

// На случай, если в MinGW нет этого типа
#ifndef STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR
struct STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR {
    DWORD Version;
    DWORD Size;
    BOOLEAN IncursSeekPenalty;
};
#endif

bool TInventory::get_all_disks_info(std::vector<DiskInfo>& disks) {
    disks.clear();

    for (int i = 0; i < 32; ++i) {
        char path[64];
        std::sprintf(path, "\\\\.\\PhysicalDrive%d", i);

        // ВАЖНО: запуск от администратора
        HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, OPEN_EXISTING, 0, nullptr);

        if (h == INVALID_HANDLE_VALUE) {
            continue;
        }

        DiskInfo info;
        info.sizeBytes = 0;
        info.model = "Unknown";
        info.type = "Unknown";

        // 1. Размер диска (геометрия)
        DISK_GEOMETRY geo{};
        DWORD bytesReturned = 0;
        BOOL okGeo = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0,
                                     &geo, sizeof(geo), &bytesReturned, nullptr);

        if (okGeo) {
            info.sizeBytes = geo.Cylinders.QuadPart * geo.TracksPerCylinder *
                             geo.SectorsPerTrack * geo.BytesPerSector;
        } else {
            CloseHandle(h);
            continue;
        }

        // 2. Модель диска (StorageDeviceProperty + STORAGE_DEVICE_DESCRIPTOR)
        STORAGE_PROPERTY_QUERY query{};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        STORAGE_DEVICE_DESCRIPTOR desc{};
        desc.Version = sizeof(desc);
        desc.Size = sizeof(desc);

        ULONG outSize = sizeof(desc) + 1024;
        std::vector<char> buffer(outSize);

        BOOL okDesc = DeviceIoControl(h, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
                                      buffer.data(), outSize, &bytesReturned, nullptr);

        if (okDesc && bytesReturned >= sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
            const STORAGE_DEVICE_DESCRIPTOR* pDesc = reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR*>(buffer.data());

            if (pDesc->ProductIdOffset != 0) {
                const char* modelStr = reinterpret_cast<const char*>(buffer.data()) + pDesc->ProductIdOffset;
                info.model = trim(std::string(modelStr));
            }
        }

        // 3. Тип диска: SSD vs HDD (Seek Penalty)
        query.PropertyId = StorageDeviceSeekPenaltyProperty;

        STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR seekDesc{};
        seekDesc.Version = sizeof(seekDesc);
        seekDesc.Size = sizeof(seekDesc);

        outSize = sizeof(seekDesc) + 64;
        buffer.resize(outSize);

        BOOL okSeek = DeviceIoControl(h, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
                                      buffer.data(), outSize, &bytesReturned, nullptr);

        if (okSeek && bytesReturned >= sizeof(STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR)) {
            const STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR* pSeek =
                reinterpret_cast<const STORAGE_DEVICE_SEEK_PENALTY_DESCRIPTOR*>(buffer.data());

            if (!pSeek->IncursSeekPenalty) {
                info.type = "SSD";
            } else {
                info.type = "HDD";
            }
        }

        if (info.sizeBytes > 0) {
            disks.push_back(info);
        }

        CloseHandle(h);
    }

    if (disks.empty()) {
        std::cerr << "No disks found. Run the program as Administrator!\n";
        return false;
    }

    // Заполняем legacy-поля (теперь id_hdd_size = "SSD 480Gb")
    if (!disks.empty()) {
        const auto& d = disks[0];
        id_hdd = d.model;

        uint64_t gb = d.sizeBytes / (1000ULL * 1000 * 1000);
        id_hdd_size = d.type + " " + std::to_string(gb) + "Gb";

        hdd_models.clear();
        for (const auto& disk : disks) {
            hdd_models.push_back(disk.model);
        }
    }

    return true;
}

bool TInventory::get_hdd() {
    std::vector<DiskInfo> dummy;
    return get_all_disks_info(dummy);
}
/*
bool TInventory::get_hdd_size() {
    std::vector<DiskInfo> dummy;
    return get_all_disks_info(dummy);
}
*/