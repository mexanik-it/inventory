#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "main.h"

std::string executeCommand(const char* command) {
    FILE* pipe = _popen(command, "r");
    if (!pipe) return "";

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

std::string getMotherboardModel() {
    return executeCommand("wmic baseboard get product");
}

std::string getMotherboardManufacturer() {
    return executeCommand("wmic baseboard get manufacturer");
}

std::string getMotherboardSerialNumber() {
    return executeCommand("wmic baseboard get serialnumber");
}

bool TInventory::get_mb() {
    // --- Производитель ---
    std::string buff = getMotherboardManufacturer();
    size_t firstLine = buff.find("\n");
    if (firstLine != std::string::npos) {
        buff = buff.substr(firstLine + 1);
    }
    size_t pos = buff.find('\n');
    if (pos != std::string::npos) {
        buff.erase(pos);
    }
    id_mb = buff;

    // --- Модель ---
    buff = getMotherboardModel();
    firstLine = buff.find("\n");
    if (firstLine != std::string::npos) {
        buff = buff.substr(firstLine + 1);
    }
    pos = buff.find('\n');
    if (pos != std::string::npos) {
        buff.erase(pos);
    }
    id_mb.append(buff);

    // Очистка от \r
    pos = id_mb.find('\r');
    if (pos != std::string::npos) {
        id_mb.erase(pos, 1);
    }
    pos = id_mb.find(0x0D);
    if (pos != std::string::npos) {
        if (pos >= 2) {
            id_mb.erase(pos - 2);
        } else {
            id_mb.erase(pos);
        }
    }

    // Удаление лишнего текста
    std::string toRemove = "COMPUTER INC.  ";
    pos = id_mb.find(toRemove);
    if (pos != std::string::npos) {
        id_mb.erase(pos, toRemove.length());
    }

    toRemove = "Technology Co., Ltd.  ";
    pos = id_mb.find(toRemove);
    if (pos != std::string::npos) {
        id_mb.erase(pos, toRemove.length());
    }

    // --- Серийный номер (через wmic, как производитель и модель) ---
    buff = getMotherboardSerialNumber();
    // Убираем заголовок "SerialNumber"
    firstLine = buff.find("\n");
    if (firstLine != std::string::npos) {
        buff = buff.substr(firstLine + 1);
    }
    // Убираем перенос строки в конце
    pos = buff.find('\n');
    if (pos != std::string::npos) {
        buff.erase(pos);
    }
    // Убираем \r
    pos = buff.find('\r');
    if (pos != std::string::npos) {
        buff.erase(pos, 1);
    }

    // Убираем лишние пробелы
    size_t first = buff.find_first_not_of(" \t");
    size_t last = buff.find_last_not_of(" \t");
    if (first != std::string::npos && last != std::string::npos) {
        sn_mb = buff.substr(first, last - first + 1);
    } else {
        sn_mb = "unknown";
    }

    return true;
}
