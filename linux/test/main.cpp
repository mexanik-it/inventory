#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::vector<std::string> get_lan_addresses() {
    std::vector<std::string> addresses;
    struct ifaddrs *ifaddr = nullptr, *ifa = nullptr;

    if (getifaddrs(&ifaddr) == -1) {
        return addresses; // пустой вектор при ошибке
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Только IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            char ip_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(sa->sin_addr), ip_str, sizeof(ip_str)) != nullptr) {
                // Пропускаем loopback (127.0.0.1) — это не LAN
                if (std::string(ip_str) != "127.0.0.1") {
                    addresses.push_back(std::string(ip_str));
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return addresses;
}

int main() {
    auto lan = get_lan_addresses();
    for (const auto &ip : lan) {
        std::cout << ip << "\n";
    }
    return 0;
}
