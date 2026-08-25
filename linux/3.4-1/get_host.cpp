#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "main.h"

/***************************************************************************************************/
/* возвращает производителя и название материнской платы                                           */
/***************************************************************************************************/


//#include <iostream>
#include <unistd.h>   // для gethostname()
#include <limits.h>  // для HOST_NAME_MAX

bool TInventory::get_host( ) {

  char hostname[HOST_NAME_MAX + 1];

    if (gethostname(hostname, sizeof(hostname)) == 0) {
        id_host =  hostname;
        // вырезаем все ненужное
        string toRemove = ".";
        size_t pos = id_host.find(toRemove);
        // Если перевод строки найден, удаляем всё начиная с него
        if (pos != std::string::npos) {
          id_host.erase(pos);
    } 

    } else {
        std::cerr << "Ошибка получения имени хоста\n";
        return false;
    }

  return true;

}


