//#include <iostream>
//#include <cstdlib>
//#include <string>
#include "main.h"

/***************************************************************************************************/
/* возвращает производителя и название материнской платы                                           */
/* данные считываются из двух системных файлов, после каждого прочтени убираем концы строк,        */
/* чтобы не было переноса - должна сформироваться одна строка (бех перехоса на новую строку        */
/* и без возврата каретки                                                                          */
/* tested on debian and alt linux                                                                  */
/***************************************************************************************************/

/* ВСЕ ВАРИАНТЫ РАБОЧИИ...

bool TInventory::get_mb( ) {
  FILE *pipe = popen("hostnamectl status | grep 'Hardware Vendor:'", "r");
  if (!pipe) {
	std::cerr << "Ошибка открытия pipe" << std::endl;
	return false;
    }

  char buffer[256];
  string str_buff;
  id_mb.erase();
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
	id_mb.append( buffer );
	id_mb = id_mb.substr(20);   // удаляем первые N символов
	id_mb.erase(id_mb.find_last_not_of(" \n\r") + 1);
    }

  pclose(pipe);

  pipe = popen("hostnamectl status | grep 'Hardware Model:'", "r");
  if (!pipe) {
	std::cerr << "Ошибка открытия pipe" << std::endl;
	return false;
    }

    str_buff.erase();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
	str_buff.append( buffer );
	str_buff = str_buff.substr(19);   // удаляем первые N символов
	id_mb.erase(id_mb.find_last_not_of(" \n\r") - 6 );
    }

  pclose(pipe);
 
  id_mb.append( str_buff.c_str() );
  return true;
}
*/
/*  ВАРИАНТ ТОЖЕ РАБОЧИЙ  */

bool TInventory::get_mb( ) {
  
  FILE *pipe = popen("cat /sys/class/dmi/id/board_vendor", "r");
  if (!pipe) {
	std::cerr << "Ошибка открытия pipe" << std::endl;
	return false;
    }

    char buffer[256];
    id_mb.erase();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
	id_mb.append( buffer );
	id_mb.erase(id_mb.find_last_not_of(" \n\r") + 1);
    }

  pclose(pipe);

  pipe = popen("cat /sys/class/dmi/id/board_name", "r"); // заново открываем поток
  if (!pipe) {
	cerr << "Ошибка открытия pipe" << endl;
	return false;
    }
  id_mb.append( " " );

  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
	id_mb.append( buffer );
	id_mb.erase(id_mb.find_last_not_of(" \n\r") + 1);
    }


    // вырезаем все ненужное
    string toRemove = "COMPUTER INC. ";
    size_t pos = id_mb.find(toRemove);
    if (pos != std::string::npos) {
        id_mb.erase(pos, toRemove.length());
    }

    toRemove = "Technology Co., Ltd. ";
    pos = id_mb.find(toRemove);
    if (pos != std::string::npos) {
        id_mb.erase(pos, toRemove.length());
    }

  pclose(pipe);
  return true;
}
