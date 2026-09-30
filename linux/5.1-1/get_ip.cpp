#include <iostream>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <iostream>
#include <cstdlib>
#include <string>

#include "main.h"


/* == вывод ==
lo: 127.0.0.1/8
eno1: 10.62.149.10/20
eno1: 10.67.11.32/16
tun0: 7.13.195.7/32
*/


bool TInventory::get_ip( ) {
    FILE* pipe = popen("ip -4 addr show | grep -oP 'inet\\s+\\K\\d+\\.\\d+\\.\\d+\\.\\d+'", "r");

    if (!pipe) {
        std::cerr << "Ошибка выполнения команды ip" << std::endl;
        return false;
    }

    //std::cout << "Локальные IP‑адреса в системе:" << std::endl;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string ip(buffer);
        ip.erase(ip.find_last_not_of(" \n\r") + 1);
        if ( ip.find("192.168.") != std::string::npos || ip.find("10.67.") != std::string::npos) {
            id_ip = ip;
            return true;
           }
        //std::cout << ip << std::endl;
    }
    pclose(pipe);
    return false;
}

