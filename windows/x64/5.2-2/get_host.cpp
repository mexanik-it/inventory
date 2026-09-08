#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include "main.h"
#include <windows.h>

bool TInventory::get_host() {
    char buffer[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD size = sizeof(buffer);

    if (GetComputerNameA(buffer, &size)) {
        id_host = std::string(buffer, size);
        return !id_host.empty();
    }

    id_host = "Не удалось получить имя компьютера";
    return false;
}
