// get_mb.cpp

#include "main.h"
#include <cctype>

// -----------------------------------------------------------------------------
// get_mb(): заполняет структуру mb (vendor, model, part, serial) из sysfs.
// Источник: /sys/class/dmi/id — те же данные, что у dmidecode, но без root-вызова.
// Тестировалось на Debian, ALT Linux, CentOS Stream.
// -----------------------------------------------------------------------------

// Читает один sysfs-файл в строку, обрезая перевод строки и пробелы по краям
static std::string readSysfs(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        return std::string();
    }

    std::string value;
    std::getline(f, value);

    size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return std::string();
    }
    size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Нормализует вендора: убирает юридические хвосты.
// "ASUSTeK COMPUTER INC." -> "ASUSTeK"
static std::string cleanVendor(const std::string& raw) {
    std::string v = raw;

    static const char* suffixes[] = {
        " Computer Inc.", " Computer Inc", " COMPUTER INC.", " COMPUTER INC",
        " Computer Systems", " Technologies Corp.", " Technology Co., Ltd.",
        " Technologies Co., Ltd.", " Co., Ltd.", " Corp.", " Inc.",
        " International", " Electronics", " Technology", " Technologies"
    };

    for (const char* s : suffixes) {
        size_t pos = v.find(s);
        if (pos != std::string::npos && pos > 0) {
            v = v.substr(0, pos);
            break;
        }
    }

    // Обрезаем пробелы по краям (на случай, если substr оставил)
    size_t first = v.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return std::string();
    }
    size_t last = v.find_last_not_of(" \t\r\n");
    return v.substr(first, last - first + 1);
}

// Проверяет, является ли значение «мусорным» (заглушкой от OEM)
static bool isPlaceholderDmi(const std::string& s) {
    if (s.empty()) {
        return true;
    }

    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const char* junk[] = {
        "to be filled by o.e.m.", "to be filled by oem", "not specified",
        "none", "default string", "system serial number", "board serial number",
        "board part number", "unknown", "0", "0000000000", "ffffffff", "null"
    };

    for (const char* j : junk) {
        if (lower == j) {
            return true;
        }
    }

    // Строки вида "AAAAA" или "00000" считаем мусором
    bool allSame = true;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] != s[0]) {
            allSame = false;
            break;
        }
    }
    return allSame;
}

bool TInventory::get_mb() {
    // 1. Vendor и Model: сначала board_*, если пусто — системные поля
    mb.vendor = readSysfs("/sys/class/dmi/id/board_vendor");
    mb.model  = readSysfs("/sys/class/dmi/id/board_name");

    if (mb.vendor.empty() && mb.model.empty()) {
        mb.vendor = readSysfs("/sys/class/dmi/id/sys_vendor");
        mb.model  = readSysfs("/sys/class/dmi/id/product_name");
    }

    // 2. Part (часто это board_asset_tag)
    mb.part = readSysfs("/sys/class/dmi/id/board_asset_tag");
    if (isPlaceholderDmi(mb.part)) {
        mb.part.clear();
    }

    // 3. Serial: board_serial, при пустоте — product_serial
    mb.serial = readSysfs("/sys/class/dmi/id/board_serial");
    if (mb.serial.empty()) {
        mb.serial = readSysfs("/sys/class/dmi/id/product_serial");
    }

    // Нормализация
    mb.vendor = cleanVendor(mb.vendor);
    mb.serial = isPlaceholderDmi(mb.serial) ? "unknown" : mb.serial;

    // Если и vendor, и model пустые — это реальная ошибка чтения DMI
    if (mb.vendor.empty() && mb.model.empty()) {
        errMessage("MB: DMI не отдал данные платы (vendor и model пусты)");
        // Возвращаем true, чтобы остальной сбор не прерывался
        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------
// mbIdString: формирует читаемую строку для тега <id_mb>
// Примеры: "ASUSTeK A68HM-K", "A68HM-K", "ASUSTeK", "unknown"
// -----------------------------------------------------------------------------
std::string mbIdString(const MbInfo& mb) {
    if (!mb.vendor.empty() && !mb.model.empty()) {
        return mb.vendor + " " + mb.model;
    }
    if (!mb.model.empty()) {
        return mb.model;
    }
    if (!mb.vendor.empty()) {
        return mb.vendor;
    }
    return "unknown";
}
