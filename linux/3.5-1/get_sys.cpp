#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "main.h"

/***************************************************************************************************/
/* возвращает название и номер операционной системы                                                */
/***************************************************************************************************/

std::map<std::string, std::string> readOSRelease() {
    std::map<std::string, std::string> osInfo;
    std::ifstream file("/etc/os-release");
    std::string line;

    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // Убираем кавычки из значения
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            osInfo[key] = value;
        }
    }
    return osInfo;
}

bool TInventory::get_sys( ) {
  auto osInfo = readOSRelease();

  if (!osInfo.empty()) {
      id_sys = osInfo["PRETTY_NAME"];
        //id_sys = osInfo["ID"];
        //id_sys = osInfo["VERSION"];
        //id_sys = osInfo["VERSION_CODENAME"];
      } else {
        std::cerr << "Файл /etc/os-release не найден или недоступен\n";
        return false;
    }

    // вырезаем все ненужное
    string toRemove = " (Laertes)";
    size_t pos = id_sys.find(toRemove);
    if (pos != std::string::npos) {
        id_sys.erase(pos, toRemove.length());
    }

    toRemove = " (december)";
    pos = id_sys.find(toRemove);
    if (pos != std::string::npos) {
        id_sys.erase(pos, toRemove.length());
    }

    toRemove = " (Autolycus)";
    pos = id_sys.find(toRemove);
    if (pos != std::string::npos) {
        id_sys.erase(pos, toRemove.length());
    }

    toRemove = "i7-8700t-";
    pos = id_sys.find(toRemove);
    if (pos != std::string::npos) {
        id_sys.erase(pos, toRemove.length());
    }


  return true;
}