#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <optional>
#include <vector>

struct MotherboardInfo {
    std::optional<std::string> vendor;
    std::optional<std::string> model;
    std::optional<std::string> serial;
};

struct CPUInfo {
    std::optional<std::string> vendor;
    std::optional<std::string> model;
    std::optional<std::string> serial;
};

struct RAMInfo {
    std::optional<std::string> vendor;
    std::optional<int> modules_count;
    std::optional<double> total_gb;
};

struct StorageDevice {
    std::string type; 
    std::optional<std::string> vendor;
    std::optional<std::string> model;
    std::optional<long long> size_gb;
    std::optional<std::string> serial;
};

struct NetworkAdapter {
    std::string name;
    std::optional<std::string> mac_address;
    std::vector<std::string> ip_addresses;
};

struct NetworkInfo {
    std::optional<std::string> hostname;
    std::vector<NetworkAdapter> adapters;
};

#endif // TYPES_H