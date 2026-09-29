#include "main.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <sys/sysinfo.h>
#include <cstdio>
#include <cerrno>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

static std::string stripQuotes(const std::string& s) {
    std::string r = trim(s);
    if (r.size() >= 2 && r.front() == '"' && r.back() == '"')
        r = r.substr(1, r.size() - 2);
    return r;
}

static std::string normalizeSpaces(const std::string& s) {
    std::string res;
    res.reserve(s.size());
    bool lastWasSpace = true;
    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!lastWasSpace) { res += ' '; lastWasSpace = true; }
        } else { res += c; lastWasSpace = false; }
    }
    if (!res.empty() && res.back() == ' ') res.pop_back();
    return res;
}

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

static bool startsWith(const std::string& s, const char* p) {
    return s.compare(0, std::char_traits<char>::length(p), p) == 0;
}

// Мусор BIOS/DMI: Unknown, Not Specified, N/A, '-', "00000000", "FFFFFFFF"
static bool isJunk(const std::string& sRaw) {
    std::string s = trim(sRaw);
    if (s.empty()) return true;

    bool allSame = true;
    for (char c : s) if (c != s[0]) { allSame = false; break; }
    if (allSame) return true;

    std::string l = toLower(s);
    static const char* junk[] = {
        "unknown", "undefined", "not specified", "not available", "none",
        "null", "no", "n/a", "na", "no dimm", "not installed", "empty", "0"
    };
    for (const char* j : junk) if (l == j) return true;
    return false;
}

// Заглушки: A1_Manufacturer0, A1_SerNum0, Array1_PartNumber0
static bool isBiosPlaceholder(const std::string& s) {
    if (isJunk(s)) return true;
    if (s.find("Manufacturer") != std::string::npos &&
        s.find_first_of("0123456789") != std::string::npos && s[0] == 'A')
        return true;
    if (s.find("PartNumber") != std::string::npos &&
        s.find("Array") != std::string::npos)
        return true;
    if (s.find("SerNum") != std::string::npos && s[0] == 'A')
        return true;
    return false;
}

static std::string valueAfterColon(const std::string& line) {
    size_t pos = line.find(':');
    return (pos != std::string::npos) ? trim(line.substr(pos + 1)) : std::string();
}

static void cleanModule(MemoryModule& m) {
    if (isBiosPlaceholder(m.manufacturer)) m.manufacturer.clear();
    if (isBiosPlaceholder(m.partNumber))   m.partNumber.clear();
    if (isBiosPlaceholder(m.serialNumber)) m.serialNumber.clear();
    if (isJunk(m.memoryType))              m.memoryType.clear();
    if (isJunk(m.speed))                   m.speed.clear();
    if (isJunk(m.locator))                 m.locator.clear();
}

static bool hasAnyInfo(const MemoryModule& m) {
    return !m.manufacturer.empty() || !m.partNumber.empty() ||
           !m.serialNumber.empty() || !m.locator.empty() ||
           !m.memoryType.empty()   || m.capacityBytes > 0;
}

/***************************************************************************************************/
/* Источник 1: decode-dimms — SPD EEPROM планок (достовернее BIOS)                                 */
/***************************************************************************************************/
static std::vector<MemoryModule> getMemoryFromDecodeDimms() {
    std::vector<MemoryModule> result;
    FILE* pipe = popen("decode-dimms 2>/dev/null", "r");
    if (!pipe) return result;

    char buffer[4096];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output += buffer;
    pclose(pipe);

    if (output.find("EEPROM") == std::string::npos) return result;

    std::istringstream iss(output);
    std::string line;
    MemoryModule current{};
    bool inModule = false;

    while (std::getline(iss, line)) {
        std::string t = trim(line);

        if (startsWith(t, "Decoding EEPROM")) {
            if (inModule && hasAnyInfo(current)) {
                cleanModule(current);
                result.push_back(current);
            }
            current = MemoryModule{};
            inModule = true;
            continue;
        }
        if (!inModule) continue;

        if (startsWith(t, "Module Manufacturer:") || startsWith(t, "Manufacturer:")) {
            std::string v = stripQuotes(valueAfterColon(t));
            if (!isJunk(v)) current.manufacturer = v;
        } else if (startsWith(t, "Part Number:")) {
            std::string v = stripQuotes(valueAfterColon(t));
            if (!isJunk(v)) current.partNumber = v;
        } else if (startsWith(t, "Serial Number:")) {
            std::string v = stripQuotes(valueAfterColon(t));
            if (!isJunk(v)) current.serialNumber = v;
        } else if (startsWith(t, "Memory Type")) {
            std::string v = stripQuotes(valueAfterColon(t));
            if (!isJunk(v)) current.memoryType = v;
        } else if (startsWith(t, "Maximum module speed") || startsWith(t, "Speed:")) {
            std::string v = stripQuotes(valueAfterColon(t));
            if (!isJunk(v) && current.speed.empty()) current.speed = v;
        } else if (startsWith(t, "Size:")) {
            std::string v = valueAfterColon(t);
            size_t sp = v.find(' ');
            if (sp != std::string::npos) {
                try {
                    uint64_t val = std::stoull(v.substr(0, sp));
                    std::string unit = v.substr(sp + 1);
                    if (unit.find("GB") != std::string::npos)
                        current.capacityBytes = val * 1024ULL * 1024 * 1024;
                    else if (unit.find("MB") != std::string::npos)
                        current.capacityBytes = val * 1024ULL * 1024;
                } catch (...) {}
            }
        }
    }

    if (inModule && hasAnyInfo(current)) { cleanModule(current); result.push_back(current); }
    return result;
}

/***************************************************************************************************/
/* Источник 2: dmidecode — SMBIOS/DMI                                                              */
/***************************************************************************************************/
static std::vector<MemoryModule> getMemoryFromDmidecode() {
    std::vector<MemoryModule> result;
    FILE* pipe = popen("dmidecode --type 17 2>/dev/null", "r");
    if (!pipe) return result;

    char buffer[4096];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output += buffer;
    pclose(pipe);
    if (output.empty()) return result;

    std::istringstream iss(output);
    std::string line;
    MemoryModule current{};
    bool inBlock = false;

    auto setField = [](std::string& out, const std::string& v) {
        if (!isJunk(v)) out = trim(v);
    };

    while (std::getline(iss, line)) {
        if (line.find("Memory Device") == 0) {
            if (inBlock && hasAnyInfo(current)) {
                cleanModule(current);
                result.push_back(current);
            }
            current = MemoryModule{};
            inBlock = true;
            continue;
        }
        if (!inBlock) continue;

        std::string t = trim(line);
        if (t.empty()) continue;

        if      (startsWith(t, "Manufacturer:"))            setField(current.manufacturer, valueAfterColon(t));
        else if (startsWith(t, "Part Number:"))             setField(current.partNumber,   valueAfterColon(t));
        else if (startsWith(t, "Serial Number:"))           setField(current.serialNumber, valueAfterColon(t));
        else if (startsWith(t, "Locator:"))                 setField(current.locator,      valueAfterColon(t));
        else if (startsWith(t, "Type Detail:")) {
            // вытаскиваем из "Synchronous Unbuffered (Unregistered)" только значимые флаги
            std::string v = toLower(valueAfterColon(t));
            if (v.find("ecc") != std::string::npos)
                current.memoryType += " ECC";
            if (v.find("registered") != std::string::npos && v.find("unregistered") == std::string::npos)
                current.memoryType += " Registered";
        }
        else if (startsWith(t, "Type:"))                    setField(current.memoryType,   valueAfterColon(t));
        else if (startsWith(t, "Configured Memory Speed:")) setField(current.speed,        valueAfterColon(t));
        else if (startsWith(t, "Configured Clock Speed:"))  setField(current.speed,        valueAfterColon(t));
        else if (startsWith(t, "Speed:") && current.speed.empty()) setField(current.speed, valueAfterColon(t));
        else if (startsWith(t, "Size:")) {
            std::string v = valueAfterColon(t);
            if (v.find("No Module") == std::string::npos) {
                size_t sp = v.find(' ');
                if (sp != std::string::npos) {
                    try {
                        uint64_t val = std::stoull(v.substr(0, sp));
                        std::string unit = v.substr(sp + 1);
                        if (unit.find("GB") != std::string::npos)
                            current.capacityBytes = val * 1024ULL * 1024 * 1024;
                        else if (unit.find("MB") != std::string::npos)
                            current.capacityBytes = val * 1024ULL * 1024;
                    } catch (...) {}
                }
            }
        }
    }

    if (inBlock && hasAnyInfo(current)) { cleanModule(current); result.push_back(current); }
    return result;
}

/***************************************************************************************************/
/* Приоритет SPD, но строки DMI дополняются данными SPD там, где BIOS молчит                       */
/***************************************************************************************************/
std::vector<MemoryModule> getMemoryModules() {
    auto spd = getMemoryFromDecodeDimms();
    auto dmi = getMemoryFromDmidecode();

    if (spd.empty()) return dmi;
    if (dmi.empty()) return spd;

    // Сопоставляем по слоту, а если Locator нет — по порядку
    for (auto& d : dmi) {
        for (const auto& s : spd) {
            bool match = !d.locator.empty() && !s.locator.empty() ? (d.locator == s.locator) : true;
            if (!match) continue;
            if (d.manufacturer.empty())  d.manufacturer  = s.manufacturer;
            if (d.partNumber.empty())    d.partNumber    = s.partNumber;
            if (d.serialNumber.empty())  d.serialNumber  = s.serialNumber;
            if (d.capacityBytes == 0)    d.capacityBytes = s.capacityBytes;
            if (d.memoryType.empty())    d.memoryType    = s.memoryType;
            if (d.speed.empty())         d.speed         = s.speed;
            break;
        }
    }
    return dmi;
}

/***************************************************************************************************/
/* id_mem_name и sn_mem — по одной строке, разделитель " / "                                       */
/***************************************************************************************************/
bool TInventory::get_mem() {
    constexpr double bytesPerGiB = 1024.0 * 1024.0 * 1024.0;

    struct sysinfo si{};
    if (sysinfo(&si) != 0) {
        std::cerr << "Error getting information about memory: " << errno << "\n";
        return false;
    }

    auto modules = getMemoryModules();

    uint64_t sumBytes = 0;
    for (const auto& m : modules) sumBytes += m.capacityBytes;

    long long gb;
    if (sumBytes > 0)
        gb = static_cast<long long>(static_cast<double>(sumBytes) / bytesPerGiB);
    else
        gb = static_cast<long long>(
            (static_cast<uint64_t>(si.totalram) * si.mem_unit) / (1024ULL * 1024 * 1024));
    id_mem = std::to_string(gb) + "Gb";

    id_mem_name.clear();
    sn_mem.clear();

    if (modules.empty()) {
        id_mem_name = "unknown";
        sn_mem = "unknown";
        return true;
    }

    for (size_t i = 0; i < modules.size(); ++i) {
        const auto& m = modules[i];
        if (i > 0) { id_mem_name += " / "; sn_mem += " / "; }

        std::string capStr = "unknown";
        if (m.capacityBytes > 0)
            capStr = std::to_string(static_cast<long long>(
                       static_cast<double>(m.capacityBytes) / bytesPerGiB)) + "Gb";

        auto addPart = [&capStr](std::string& desc, const std::string& v) {
            std::string n = normalizeSpaces(v);
            if (n.empty()) return;
            if (!desc.empty()) desc += ' ';
            desc += n;
        };

        std::string desc;
        if (!m.partNumber.empty()) {           // есть парт-номер — это лучший идентификатор
            addPart(desc, m.manufacturer);
            addPart(desc, m.partNumber);
        } else {                               // парт-номера нет — собираем из того, что отдаёт BIOS
            addPart(desc, m.manufacturer);
            addPart(desc, m.memoryType);
            addPart(desc, m.speed);
        }
        addPart(desc, m.locator);              // слот всегда полезен: планки становятся различимы

        if (desc.empty())
            id_mem_name += "Unknown (" + capStr + ")";
        else
            id_mem_name += desc + " (" + capStr + ")";

        sn_mem += isJunk(m.serialNumber) ? "unknown" : normalizeSpaces(m.serialNumber);
    }

    return true;
}
