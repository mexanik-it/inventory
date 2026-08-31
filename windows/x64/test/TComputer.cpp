#include "TComputer.h"
#include <locale.h>

TComputer::TComputer() { setlocale(LC_ALL, ""); }

const MotherboardInfo& TComputer::get_mb() {
    if (!_mb.vendor.has_value()) { _collector_mb.collect(_mb); } return _mb;
}
const CPUInfo& TComputer::get_cpu() {
    if (!_cpu.vendor.has_value()) { _collector_cpu.collect(_cpu); } return _cpu;
}
const RAMInfo& TComputer::get_ram() {
    if (!_ram.total_gb.has_value()) { _collector_ram.collect(_ram); } return _ram;
}
const std::vector<StorageDevice>& TComputer::get_storage() {
    if (_storage.empty()) { _collector_storage.collect(_storage); } return _storage;
}
const NetworkInfo& TComputer::get_network() {
    if (!_network.hostname.has_value() && _network.adapters.empty()) { _collector_network.collect(_network); } return _network;
}

std::unordered_map<std::string, std::any> TComputer::get_all() {
    std::unordered_map<std::string, std::any> report;
    report["Motherboard"] = get_mb();
    report["CPU"] = get_cpu();
    report["RAM"] = get_ram();
    report["Storage"] = get_storage();
    report["Network"] = get_network();
    return report;
}