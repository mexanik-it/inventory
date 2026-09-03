#include "main.h"

/***************************************************************************************************/
/* возвращает запрашивает номер кабинета и название здания                                         */
/***************************************************************************************************/

bool TInventory::get_other( ) {

  std::string full_name;

  std::cout << "Enter number of the room: ";
  getline( std::cin, id_office );

  std::cout << "Enter the building: ";
  getline( std::cin, id_structure );

  std::cout << "Input inventory number: ";
  getline( std::cin, id_inv_number );

  return true;

}


