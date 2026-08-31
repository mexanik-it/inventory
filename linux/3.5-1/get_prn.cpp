#include "main.h"
#include <string>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <vector>
#endif

// --- trim: убрать пробелы по краям ---
std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

// --- to_lower: для нечувствительного поиска ---
std::string to_lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

// --- Получение имени принтера по умолчанию (кроссплатформенно) ---
std::string get_default_printer() {
#ifdef _WIN32
    DWORD size = 0;
    // Первый вызов: получаем требуемый размер буфера
    if (!GetDefaultPrinterA(nullptr, &size)) {
        return "";
    }

    // size включает завершающий '\0', поэтому выделяем ровно size байт
    std::vector<char> buffer(size);
    if (!GetDefaultPrinterA(buffer.data(), &size)) {
        return "";
    }

    // Возвращаем строку без завершающего '\0'
    return std::string(buffer.data(), size - 1);
#else
    // Linux: сначала пробуем переменную окружения PRINTER
    const char* env = std::getenv("PRINTER");
    if (env && *env != '\0') {
        return trim(std::string(env));
    }

    // Fallback: lpstat -d (CUPS)
    FILE* fp = popen("lpstat -d 2>/dev/null", "r");
    if (!fp) return "";

    char buf[256] = {};
    if (!fgets(buf, sizeof(buf), fp)) {
        pclose(fp);
        return "";
    }
    pclose(fp);

    const char* prefix = "system default destination: ";
    const char* p = std::strstr(buf, prefix);
    if (!p) return "";

    p += static_cast<int>(std::strlen(prefix));
    size_t len = std::strlen(p);

    // Убираем возможные \n и \r в конце
    while (len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r')) {
        --len;
    }

    return trim(std::string(p, len));
#endif
}

struct PrinterModel {
    std::string part1;
    std::string part2;
    std::string full_name;
};

static const auto known_models = std::to_array<PrinterModel>({
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
});

bool TInventory::get_prn() {
    auto printer = get_default_printer();
    if (printer.empty()) {
        return false;
    }

    id_prn = printer;

    const std::string printer_lower = to_lower(printer);

    for (const auto& m : known_models) {
        const std::string p1 = to_lower(m.part1);
        const std::string p2 = to_lower(m.part2);

        if (printer_lower.find(p1) != std::string::npos &&
            printer_lower.find(p2) != std::string::npos) {
            id_prn = m.full_name;
            return true;
        }
    }

    return !printer.empty();
}
