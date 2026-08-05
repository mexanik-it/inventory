#include "main.h"
#include <cstdlib>
#include <string>
#include <cstdlib>
#include <string>
#include <iostream>

bool TInventory::write_to_lan() {

    const std::string src  = id_filename;
    const std::string dest = id_filename;

    // -N = анонимный доступ; если нужен пользователь — убери -N и добавь -U user%pass
    std::string cmd = "smbclient //10.67.2.33/inventory-base -N -c \"put " + src + " " + dest + "\"";

    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[INFO] Файл загружен через smbclient.\n";
    } else {
        std::cerr << "[ERROR] smbclient вернул код " << ret << "\n";
        return 1;
    }
    return 0;
}

