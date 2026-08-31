#include "main.h"
#include <windows.h>
#include <vector>
#include <string>

bool TInventory::get_cpu() {
    HKEY hKey;
    DWORD type = REG_SZ;
    char buffer[256] = {0};
    DWORD bufferSize = sizeof(buffer);

    // Открываем ключ реестра
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    // Читаем ProcessorNameString
    if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, &type,
                         reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
        id_cpu = buffer;
    } else {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);

    // Список подстрок для удаления
    const std::vector<std::string> toRemove = {
        "Core(TM) ",
        "CPU @ ",
        " Dual Core Processor ",
        "(R)",
        "(TM)",
        "@",
        "Processor"
    };

    // Удаляем все указанные подстроки (в том числе повторяющиеся)
    for (const auto& sub : toRemove) {
        size_t pos;
        while ((pos = id_cpu.find(sub)) != std::string::npos) {
            id_cpu.erase(pos, sub.length());
        }
    }

    // Функция для удаления лишних пробелов
    auto trim_spaces = [](std::string& s) {
        std::string result;
        bool lastWasSpace = true;
        for (char c : s) {
            if (c == ' ') {
                if (!lastWasSpace) {
                    result += ' ';
                    lastWasSpace = true;
                }
            } else {
                result += c;
                lastWasSpace = false;
            }
        }
        s = result;

        size_t start = s.find_first_not_of(' ');
        if (start == std::string::npos) {
            s.clear();
            return;
        }
        size_t end = s.find_last_not_of(' ');
        s = s.substr(start, end - start + 1);
    };
    trim_spaces(id_cpu);

    return true;
}
