// get_mem.cpp
//------------

// get_mem.cpp
#include "main.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sys/sysinfo.h>
#include <sstream>
#include <algorithm>

namespace {

int normalize_ram_gb(int raw) {
    constexpr int sizes[] = {2, 4, 6, 8, 10, 12, 16, 18, 20, 24, 28, 32, 48, 64, 96};
    for (int s : sizes) {
        if (raw <= s)
            return s;
    }
    return raw;
}

std::vector<MemInfo> parse_dmidecode_memory(const std::string& output) {
    std::vector<MemInfo> modules;
    std::istringstream stream(output);
    std::string line;

    MemInfo current;
    bool haveModule = false;

    while (std::getline(stream, line)) {
        if (line.find("Memory Device") != std::string::npos) {
            if (haveModule && current.capacityBytes > 0) {
                modules.push_back(current);
            }
            current = MemInfo{};
            haveModule = true;
            continue;
        }

        // Size: "2 GiB", "16 GB", "No Module Installed"
        if (line.find("\tSize:") != std::string::npos) {
            std::string val = line.substr(line.find(':') + 1);
            if (val.find("No Module Installed") != std::string::npos) {
                current.capacityBytes = 0;
            } else {
                std::istringstream vs(val);
                long long size = 0;
                std::string unit;
                vs >> size >> unit;
                // Поддержка и десятичных (GB, MB), и двоичных (GiB, MiB) единиц
                if (unit == "GB" || unit == "GiB")
                    current.capacityBytes = static_cast<uint64_t>(size) * 1'000'000'000ULL;
                else if (unit == "MB" || unit == "MiB")
                    current.capacityBytes = static_cast<uint64_t>(size) * 1'000'000ULL;
                else if (unit == "kB" || unit == "KiB")
                    current.capacityBytes = static_cast<uint64_t>(size) * 1'000ULL;
                else if (unit == "bytes")
                    current.capacityBytes = static_cast<uint64_t>(size);
            }
            continue;
        }

        // Manufacturer → vendor
        if (line.find("\tManufacturer:") != std::string::npos) {
            std::string val = line.substr(line.find(':') + 1);
            auto first = val.find_first_not_of(" \t");
            auto last = val.find_last_not_of(" \t");
            if (first != std::string::npos)
                current.vendor = val.substr(first, last - first + 1);
            else
                current.vendor = "unknown";
            continue;
        }

        // Part Number → part
        if (line.find("\tPart Number:") != std::string::npos) {
            std::string val = line.substr(line.find(':') + 1);
            auto first = val.find_first_not_of(" \t");
            auto last = val.find_last_not_of(" \t");
            if (first != std::string::npos)
                current.part = val.substr(first, last - first + 1);
            else
                current.part = "unknown";
            continue;
        }

        // Serial Number → serial
        if (line.find("\tSerial Number:") != std::string::npos) {
            std::string val = line.substr(line.find(':') + 1);
            auto first = val.find_first_not_of(" \t");
            auto last = val.find_last_not_of(" \t");
            if (first != std::string::npos)
                current.serial = val.substr(first, last - first + 1);
            else
                current.serial = "unknown";
            continue;
        }

        // Type
        if (line.find("\tType:") != std::string::npos) {
            std::string val = line.substr(line.find(':') + 1);
            auto first = val.find_first_not_of(" \t");
            auto last = val.find_last_not_of(" \t");
            if (first != std::string::npos)
                current.type = val.substr(first, last - first + 1);
            else
                current.type = "unknown";
            continue;
        }
    }

    if (haveModule && current.capacityBytes > 0) {
        modules.push_back(current);
    }

    return modules;
}

} // namespace

bool TInventory::get_mem() {
    struct sysinfo info{};
    if (sysinfo(&info) != 0) {
        errMessage("sysinfo failed: cannot retrieve system memory information");
        return false;
    }

    const double ram_gb_raw =
        static_cast<double>(info.totalram) / 1'000'000'000.0;
    int total_gb = normalize_ram_gb(static_cast<int>(std::round(ram_gb_raw)));

    memModules.clear();

    FILE* fp = popen("dmidecode -t memory 2>/dev/null", "r");
    if (fp) {
        std::string dmidecode_output;
        char buf[4096];
        while (fgets(buf, sizeof(buf), fp)) {
            dmidecode_output += buf;
        }
        pclose(fp);

        if (!dmidecode_output.empty()) {
            memModules = parse_dmidecode_memory(dmidecode_output);
        }
    }

    // id_mem
    std::ostringstream oss_mem;
    oss_mem << total_gb << "Gb";
    id_mem = oss_mem.str();

    // id_mem_name — "A1_Manufacturer0 Array1_PartNumber0 (2Gb) / ..."
    if (memModules.empty()) {
        id_mem_name = "unknown";
    } else {
        std::ostringstream oss;
        for (size_t i = 0; i < memModules.size(); ++i) {
            if (i > 0)
                oss << " / ";

            const auto& m = memModules[i];

            if (m.vendor != "unknown" && m.vendor != "Not Specified") {
                oss << m.vendor << " ";
            }
            if (m.part != "unknown" && m.part != "Not Specified") {
                oss << m.part;
            }

            if (m.capacityBytes > 0) {
                double gb = static_cast<double>(m.capacityBytes) / 1'000'000'000.0;
                oss << " (" << static_cast<int>(std::round(gb)) << "Gb)";
            }
        }
        id_mem_name = oss.str();
    }

    return true;
}
