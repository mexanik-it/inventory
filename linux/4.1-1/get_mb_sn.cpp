#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <algorithm>        // <-- ЭТОГО НЕ ХВАТАЛО
#include <cctype>           // для std::isspace

#include "main.h"

std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

bool TInventory::get_mb_sn( ) {
    try {
        std::string out = exec_command("dmidecode -s baseboard-serial-number");

        // Удаляем все пробельные символы (пробел, табуляция, \n, \r и т.д.)
        out.erase(std::remove_if(out.begin(), out.end(),
                [](unsigned char ch){ return std::isspace(ch); }), out.end());

        if (!out.empty()) {
            std::cout << "Serial number: " << out << "\n";
            sn_mb = out;
        } else {
            std::cout << "No serial number found or access denied.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return true;
}
