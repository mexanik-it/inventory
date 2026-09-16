#include "main.h"
#include <cstdlib>
#include <string>
#include <cstdlib>
#include <string>
#include <iostream>

/*
bool TInventory::write_to_lan() {

    const std::string src  = id_filename;
    const std::string dest = id_filename;

    // -N = анонимный доступ; если нужен пользователь — убери -N и добавь -U user%pass
    std::string cmd = "smbclient //10.67.2.33/inventory-base -N -c \"put " + src + " " + dest + "\"";

    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[INFO] Файл загружен через smbclient.\n";
    } else {
        errMessage("  SMBclient вернул код " + std::to_string(ret) + "\n");
        //errMessage( string( "  smbclient вернул код " + ret + "\n" ) ;
        return 1;
    }
    return 0;
} */

bool TInventory::write_to_lan() {
    const std::string src  = id_filename;
    const std::string dest = id_filename;

    // Формируем базовую команду
    std::string baseCmd = "smbclient //10.67.2.33/inventory-base -N -c \"put " + src + " " + dest + "\"";

#ifdef _WIN32
    // Для Windows: перенаправляем stderr в NUL
    std::string cmd = baseCmd + " 2>NUL";
#else
    // Для Linux (ALT Linux): перенаправляем stderr в /dev/null
    std::string cmd = baseCmd + " 2>/dev/null";
#endif

    int ret = std::system(cmd.c_str());

    if (ret == 0) {
        okMessage("Файл загружен через smbclient."); // Лучше использовать okMessage
        //std::cout << "[INFO] Файл загружен через smbclient.\n";
    } else {
        errMessage("SMBclient вернул код " + std::to_string(ret));
        return false; // Возвращай bool, раз функция объявлена как bool
    }
    return true;
}


