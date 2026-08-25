#include "main.h"

//#pragma comment(lib, "ws2_32.lib")

/* == вывод ==
lo: 127.0.0.1/8
eno1: 10.62.149.10/20
eno1: 10.67.11.32/16
tun0: 7.13.195.7/32
*/


bool TInventory::get_ip( ) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return "WSAStartup failed";
    }

    char hostname[256];
    result = gethostname(hostname, sizeof(hostname));
    if (result == SOCKET_ERROR) {
        WSACleanup();
        return "gethostname failed";
    }

    struct hostent* hostEntry = gethostbyname(hostname);
    if (!hostEntry) {
        WSACleanup();
        return "gethostbyname failed";
    }

    // Получаем первый IP-адрес
    struct in_addr** addr_list = (struct in_addr**)hostEntry->h_addr_list;
    std::string ip = inet_ntoa(*addr_list[0]);

    WSACleanup();
    id_ip = ip;
    return true;
}

