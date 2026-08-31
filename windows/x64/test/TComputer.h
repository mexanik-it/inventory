#ifndef TCOMPUTER_H
#define TCOMPUTER_H

#include <unordered_map>
#include <any>
#include "Types.h"
#include "CollectorMB.h"
#include "CollectorCPU.h"
#include "CollectorRAM.h"
#include "CollectorStorage.h"
#include "CollectorNetwork.h"

class TComputer {
private:
    MotherboardInfo _mb{};
    CPUInfo _cpu{};
    RAMInfo _ram{};
    std::vector<StorageDevice> _storage{};
    NetworkInfo _network{};

    CollectorMB _collector_mb;
    CollectorCPU _collector_cpu;
    CollectorRAM _collector_ram;
    CollectorStorage _collector_storage;
    CollectorNetwork _collector_network;

public:
    TComputer();
    const MotherboardInfo& get_mb();
    const CPUInfo& get_cpu();
    const RAMInfo& get_ram();
    const std::vector<StorageDevice>& get_storage();
    const NetworkInfo& get_network();
    std::unordered_map<std::string, std::any> get_all();
};

#endif