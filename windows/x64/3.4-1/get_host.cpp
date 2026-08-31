#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "main.h"

/***************************************************************************************************/
/* возвращает производителя и название материнской платы                                           */
/***************************************************************************************************/


#include <iostream>
#include <windows.h>

std::string getComputerNameFromEnv() {
    const char* name = std::getenv("COMPUTERNAME");
    if (name != nullptr) {
        return std::string(name);
    } else {
        return "Не удалось получить имя компьютера";
    }
}

bool TInventory::get_host( ) {
  id_host = getComputerNameFromEnv();
  return true;

}


