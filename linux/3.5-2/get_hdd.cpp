#include "main.h"

/***************************************************************************************************/
/* returns the hard drive type (SSD/HDD) and size, and if the disk brand is known, both the type   */
/* and size are taken from the disk name                                                           */
/*-------------------------------------------------------------------------------------------------*/
/***************************************************************************************************/
/* получаем название жесткого диска и по возможности его производителя, если название диска есть   */
/* во встроенном списке, и официальные марка название и производитель берутся оотуда в             */
/* соответствии с заводской спецификацией                                                          */
/*-------------------------------------------------------------------------------------------------*/
/*                  модуль получения производителя жесткого диска и название модели                */

bool TInventory::get_hdd( ) {

 char buff[50] = "0";
  
 FILE *pipein = popen("lsblk -o MODEL -d", "r");
 fread( buff, 50, 50 , pipein);
 replace(buff, '\n', ' ');
  

 id_hdd = buff;
 id_hdd.erase( 0, 6);

 size_t pos = id_hdd.find("DT01ACA100");
    if (pos != std::string::npos) {
        id_hdd = "TOSHIBA DT01ACA100";
        return true;
	} 


    // вырезаем все ненужное
    //string toRemove = "COMPUTER INC.  ";
    //size_t pos = id_mb.find(0x0A);
    //if (pos != std::string::npos) {
    //    id_mb.erase(pos);
    //}

 return true;

}