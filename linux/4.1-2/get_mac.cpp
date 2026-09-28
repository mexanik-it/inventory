#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>

#include "main.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <dirent.h>

// 


bool TInventory::get_mac( ) {

 char buff[150] = "0";
  
 FILE *pipein = popen("cat /sys/class/net/*/address", "r");
 fread( buff, 150, 150 , pipein);
 replace(buff, '\n', ' ');
  

 id_mac = buff;
 id_mac.erase( 17 );

    for (char& c : id_mac) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
 return true;
}
