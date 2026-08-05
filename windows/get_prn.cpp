#include "main.h"
#include <windows.h>
#include <winspool.h>
#include <algorithm>
#include <cctype>
#include <vector>

// --- Вспомогательные функции (можно вынести в отдельный .h) ---
static inline std::string trim(const std::string& s) {
    const char* ws = " \t\n\r";
    size_t first = s.find_first_not_of(ws);
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

static inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::string get_default_printer() {
    DWORD size = 0;
    GetDefaultPrinterA(nullptr, &size);
    if (size <= 1) return "";

    std::vector<char> buf(size);
    if (!GetDefaultPrinterA(buf.data(), &size)) {
        return "";
    }
    return trim(std::string(buf.data(), size - 1));
}

// Таблица моделей
static const std::array<PrinterModel, 17> known_models = {{
    {"HP",     "m14",     "HP Laserjet M14-M17"},
    {"HP",     "1002",    "HP LaserJet P1002"},
    {"HP",     "1005",    "HP LaserJet P1005"},
    {"HP",     "1010",    "HP LaserJet 1010"},
    {"HP",     "1012",    "HP LaserJet 1012"},
    {"HP",     "1015",    "HP LaserJet 1015"},
    {"HP",     "1020",    "HP LaserJet 1020"},
    {"HP",     "404",     "HP LaserJet Pro M404dn"},
    {"HP",     "428",     "HP LaserJet Pro M428fdw"},
    {"HP",     "426",     "HP LaserJet Pro M426fdn"},
    {"HP",     "M1132",   "HP LaserJet Pro M1132"},
    {"Pantum", "6500",    "Pantum M6550"},
    {"Pantum", "5000",    "Pantum M5000"},
    {"Pantum", "5100DN",  "Pantum BP5100DN"},
    {"Pantum", "5100ADN", "Pantum BP5100ADN"},
    {"Samsung", "4824",   "Samsung SCX-4824FN"},
    {"Samsung", "4833",   "Samsung SCX-4833FD"},
}};

// Реализация метода
bool TInventory::get_prn() {
    auto printer = get_default_printer();
    if (printer.empty()) {
        return false;
    }

    id_prn = printer; // fallback: оставляем как есть, если не распознали
    const std::string needle = to_lower(printer);

    for (const auto& m : known_models) {
        const std::string p1 = to_lower(m.part1);
        const std::string p2 = to_lower(m.part2);

        // Ищем обе части, но допускаем, что между ними или вокруг них есть другие символы
        if (needle.find(p1) != std::string::npos && needle.find(p2) != std::string::npos) {
            id_prn = m.full_name;
            return true; // Модель опознана
        }
    }

    return false; // Не опознан
}