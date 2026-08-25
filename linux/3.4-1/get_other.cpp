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

bool TInventory::get_other( ) {

  string full_name;

  cout << "Введите кабинет: "<< std::flush;;
  getline( cin, id_office );

  //cin.ignore(); // очищаем буфер от символа \n после ввода числа

  cout << "Введите здание: "<< std::flush;;
  getline( cin, id_structure );

  //cout << "Имя: " << full_name << ", возраст: " << age << endl;
  return true;

}


