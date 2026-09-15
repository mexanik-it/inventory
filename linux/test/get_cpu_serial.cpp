#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// Убираем пробелы в начале и конце
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string get_cpuid_from_cpuinfo() {
    std::ifstream f("/proc/cpuinfo");
    if (!f.is_open()) return "";

    std::string line;
    std::string model_name, cpu_cores, siblings, physical_id;
    int physical_ids_seen = 0;
    std::string last_physical_id = "-1";

    while (std::getline(f, line)) {
        // Ищем нужные поля
        auto pos = line.find(':');
        if (pos == std::string::npos) {
            // Пустая строка — граница между процессорами
            if (line.empty()) continue;
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));

        if (key == "model name" && model_name.empty()) {
            model_name = val;
        } else if (key == "cpu cores" && cpu_cores.empty()) {
            cpu_cores = val;
        } else if (key == "siblings" && siblings.empty()) {
            siblings = val;
        } else if (key == "physical id") {
            if (val != last_physical_id) {
                last_physical_id = val;
                physical_ids_seen++;
            }
        }
    }

    if (model_name.empty()) return "";

    // Собираем уникальную строку
    std::ostringstream oss;
    oss << model_name;
    oss << "; cores=" << cpu_cores;
    oss << "; threads=" << siblings;
    oss << "; sockets=" << physical_ids_seen;
    return oss.str();
}

int main() {
    std::string id = get_cpuid_from_cpuinfo();
    if (!id.empty()) {
        std::cout << "CPU ID: " << id << "\n";
    } else {
        std::cout << "Failed to read /proc/cpuinfo\n";
    }
    return 0;
}
