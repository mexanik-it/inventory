#include "main.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <sys/sysinfo.h>
#include <cstdio>
#include <cerrno>

static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

static std::string normalizeSpaces(const std::string& s) {
    std::string res;
    res.reserve(s.size());
    bool lastWasSpace = true;
    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!lastWasSpace) { res += ' '; lastWasSpace = true; }
        } else {
            res += c; lastWasSpace = false;
        }
    }
    if (!res.empty() && res.back() == ' ') res.pop_back();
    return res;
}

static bool isDummySerial(const std::string& s) {
    if (s.empty()) return true;
    char first = s[0];
    bool allSame = true;
    for (char c : s) { if (c != first) { allSame = false; break; } }
    if (allSame) return true;
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "none" || lower == "null" || lower == "0" ||
        lower == "00000000" || lower == "ffffffff" || lower == "no dimm")
        return true;
    return false;
}

// Детекция заглушек BIOS вида A1_Manufacturer0, Array1_PartNumber0, A1_SerNum0
static bool isBiosPlaceholder(const std::string& s) {
    if (s.empty()) return true;
    // Паттерны: A\d_Manufacturer\d, Array\d_PartNumber\d, A\d_SerNum\d
    if (s.find("Manufacturer") != std::string::npos &&
        s.find_first_of("0123456789") != std::string::npos &&
        s.find("A") == 0)
        return true;
    if (s.find("PartNumber") != std::string::npos &&
        s.find("Array") != std::string::npos)
        return true;
    if (s.find("SerNum") != std::string::npos && s.find("A") == 0)
        return true;
    // Общий паттерн: что-то вида Xx_Yyyy0
    if (s.find("_") != std::string::npos) {
        // Если строка содержит _ и заканчивается цифрой — подозрительно
        // но пропускаем реальные парт-номера (они обычно без _)
        // Точнее: если это похоже на шаблон BIOS
        size_t lastDigit = s.find_last_of("0123456789");
        size_t lastUnderscore = s.find_last_of("_");
        if (lastDigit == s.length() - 1 && lastUnderscore != std::string::npos &&
            lastUnderscore < lastDigit) {
            // Проверяем что перед _ есть буква+цифра (A1_)
            if (lastUnderscore >= 2 &&
                std::isalpha(s[0]) &&
                std::isdigit(s[1])) {
                return true;
            }
        }
    }
    return false;
}

/***************************************************************************************************/
/* Источник 1: decode-dimms (читает SPD EEPROM напрямую через I2C)                                  */
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

    if (output.empty()) return result;

    std::istringstream iss(output);
    std::string line;
    MemoryModule current{};
    bool inModule = false;

    while (std::getline(iss, line)) {
        std::string trimmed = trim(line);

        // Начало блока модуля
        if (trimmed.find("Decoding EEPROM") == 0) {
            if (inModule) {
                if (!current.manufacturer.empty() || !current.partNumber.empty())
                    result.push_back(current);
                current = MemoryModule{};
            }
            inModule = true;
            continue;
        }

        if (!inModule) continue;

        // Manufacturer
        if (trimmed.find("Manufacturer:") == 0 || trimmed.find("Module Manufacturer:") == 0) {
            std::string val = trim(trimmed.substr(trimmed.find(":") + 1));
            // decode-dimms иногда добавляет (конкретный hex)
            if (val.empty() || val == "Unknown" || val == "Undefined")
                current.manufacturer.clear();
            else
                current.manufacturer = val;
        }

        // Part Number
        if (trimmed.find("Part Number:") == 0) {
            std::string val = trim(trimmed.substr(trimmed.find(":") + 1));
            if (val.empty() || val == "Unknown")
                current.partNumber.clear();
            else
                current.partNumber = val;
        }

        // Serial Number
        if (trimmed.find("Serial Number:") == 0) {
            std::string val = trim(trimmed.substr(trimmed.find(":") + 1));
            if (!val.empty() && val != "Unknown")
                current.serialNumber = val;
        }

        // Size: decode-dimms пишет "Size: 2048 MB" или "Size: 4096 MB"
        if (trimmed.find("Size:") == 0) {
            std::string val = trim(trimmed.substr(trimmed.find(":") + 1));
            size_t spacePos = val.find(' ');
            if (spacePos != std::string::npos) {
                try {
                    uint64_t sizeVal = std::stoull(val.substr(0, spacePos));
                    std::string unit = val.substr(spacePos + 1);
                    if (unit.find("MB") != std::string::npos)
                        current.capacityBytes = sizeVal * 1024ULL * 1024;
                    else if (unit.find("GB") != std::string::npos)
                        current.capacityBytes = sizeVal * 1024ULL * 1024 * 1024;
                } catch (...) {}
            }
        }
    }

    if (inModule && (!current.manufacturer.empty() || !current.partNumber.empty()))
        result.push_back(current);

    return result;
}

/***************************************************************************************************/
/* Источник 2: dmidecode (читает SMBIOS/DMI таблицу)                                                 */
/***************************************************************************************************/

static std::vector<MemoryModule> getMemoryFromDmidecode() {
    std::vector<MemoryModule> result;

    FILE* pipe = popen("dmidecode --type memory 2>/dev/null", "r");
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
    bool inMemoryBlock = false;

    auto parseField = [](std::string l, const std::string& prefix, std::string& out) {
        l = trim(l);
        if (l.find(prefix) == 0) {
            size_t pos = l.find(":");
            if (pos != std::string::npos && pos + 1 < l.size())
                out = trim(l.substr(pos + 1));
        }
    };

    while (std::getline(iss, line)) {
        if (line.find("Memory Device") == 0) {
            if (inMemoryBlock &&
                (!current.manufacturer.empty() || !current.partNumber.empty()
                 || current.capacityBytes > 0)) {
                // Фильтруем заглушки BIOS
                if (isBiosPlaceholder(current.manufacturer))
                    current.manufacturer.clear();
                if (isBiosPlaceholder(current.partNumber))
                    current.partNumber.clear();
                if (isBiosPlaceholder(current.serialNumber))
                    current.serialNumber.clear();

                result.push_back(current);
                current = MemoryModule{};
            }
            inMemoryBlock = true;
            continue;
        }

        if (!inMemoryBlock) continue;
        if (trim(line).empty()) continue;

        parseField(line, "Manufacturer", current.manufacturer);
        parseField(line, "Part Number", current.partNumber);
        parseField(line, "Serial Number", current.serialNumber);

        if (trim(line).find("Size") == 0) {
            std::string sizeStr = trim(line.substr(line.find(":") + 1));
            if (sizeStr.find("No Module") == std::string::npos && !sizeStr.empty()) {
                size_t spacePos = sizeStr.find(' ');
                if (spacePos != std::string::npos) {
                    try {
                        uint64_t val = std::stoull(sizeStr.substr(0, spacePos));
                        std::string unit = sizeStr.substr(spacePos + 1);
                        if (unit.find("GB") != std::string::npos ||
                            unit.find("GiB") != std::string::npos)
                            current.capacityBytes = val * 1024ULL * 1024 * 1024;
                        else if (unit.find("MB") != std::string::npos ||
                                 unit.find("MiB") != std::string::npos)
                            current.capacityBytes = val * 1024ULL * 1024;
                    } catch (...) {}
                }
            }
        }
    }

    if (inMemoryBlock &&
        (!current.manufacturer.empty() || !current.partNumber.empty()
         || current.capacityBytes > 0)) {
        if (isBiosPlaceholder(current.manufacturer))
            current.manufacturer.clear();
        if (isBiosPlaceholder(current.partNumber))
            current.partNumber.clear();
        if (isBiosPlaceholder(current.serialNumber))
            current.serialNumber.clear();
        result.push_back(current);
    }

    return result;
}

/***************************************************************************************************/
/* Получение модулей: сначала decode-dimms, потом dmidecode                                         */
/***************************************************************************************************/

std::vector<MemoryModule> getMemoryModules() {
    // Сначала пробуем decode-dimms (реальные SPD данные)
    auto modules = getMemoryFromDecodeDimms();
    if (!modules.empty())
        return modules;

    // Если не получилось — dmidecode (DMI таблица)
    return getMemoryFromDmidecode();
}

/***************************************************************************************************/
/* Главная функция                                                                                  */
/***************************************************************************************************/

bool TInventory::get_mem() {
    constexpr double bytesPerGiB = 1024.0 * 1024.0 * 1024.0;

    struct sysinfo si{};
    if (sysinfo(&si) != 0) {
        std::cerr << "Error getting information about memory: " << errno << "\n";
        return false;
    }

    int intGB = static_cast<int>(
        (static_cast<uint64_t>(si.totalram) * si.mem_unit) /
        (1024ULL * 1024 * 1024));
    id_mem = std::to_string(intGB) + "Gb";

    auto modules = getMemoryModules();

    id_mem_name.clear();
    sn_mem.clear();

    if (modules.empty()) {
        id_mem_name = "unknown";
        sn_mem = "unknown";
        return true;
    }

    for (size_t i = 0; i < modules.size(); ++i) {
        const auto& mod = modules[i];

        if (i > 0) id_mem_name += " / ";

        std::string capStr;
        if (mod.capacityBytes > 0) {
            double gb = static_cast<double>(mod.capacityBytes) / bytesPerGiB;
            capStr = std::to_string(static_cast<long long>(gb)) + "Gb";
        } else {
            capStr = "unknown";
        }

        std::string man = mod.manufacturer.empty()
            ? "Unknown"
            : normalizeSpaces(mod.manufacturer);
        std::string pn = normalizeSpaces(mod.partNumber);

        if (!pn.empty()) {
            id_mem_name += man + " " + pn + " (" + capStr + ")";
        } else {
            id_mem_name += man + " (" + capStr + ")";
        }

        if (i > 0) sn_mem += " / ";

        std::string sn = trim(mod.serialNumber);
        if (isDummySerial(sn)) {
            sn = "unknown";
        }
        sn_mem += sn;
    }

    return true;
}
