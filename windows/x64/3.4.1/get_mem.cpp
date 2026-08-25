#include "main.h"
#include <cmath>
#include <iostream>
#include <windows.h>

/***************************************************************************************************/
/* возвращает общее количество оперативной памяти в Gb                                             */
/***************************************************************************************************/

#include <windows.h>
#include <string>
#include <cmath>
#include <iostream>

bool TInventory::get_mem() {
    MEMORYSTATUSEX memoryInfo{};  // обнуляем всю структуру
    memoryInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memoryInfo)) {
        std::cerr << "Error getting information about memory: "
                  << GetLastError() << "\n";
        return false;
    }

    // Используем 1 GiB = 1024^3 байт (стандарт для ОС)
    constexpr double bytesPerGiB = 1024.0 * 1024.0 * 1024.0;

    double totalMemoryGiB = static_cast<double>(memoryInfo.ullTotalPhys) / bytesPerGiB;

    // Вариант 1: округление до целого числа гигабайт
    int intGB = static_cast<int>(std::round(totalMemoryGiB));
    id_mem = std::to_string(intGB) + "Gb";

    // Вариант 2 (альтернатива): оставить один знак после запятой
    // char buffer[64];
    // std::snprintf(buffer, sizeof(buffer), "%.1f GiB", totalMemoryGiB);
    // id_mem = buffer;

    return true;
}
