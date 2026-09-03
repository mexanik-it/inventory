#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

struct LanInfo {
    std::string name;            // FriendlyName (как раньше)
    std::string interface_name;  // Description (название интерфейса)
    std::string mac;
    std::string ipv4;
    bool is_active = false;
};

static std::string sockaddr_to_ipv4(const SOCKADDR *addr) {
    if (!addr || addr->sa_family != AF_INET)
        return "";
    const sockaddr_in *sin = reinterpret_cast<const sockaddr_in*>(addr);
    return inet_ntoa(sin->sin_addr);
}

// Конвертация wchar_t* -> std::string
static std::string wchar_to_string(const wchar_t *w) {
    if (!w) return "Unknown";
    std::wstring ws(w);
    return std::string(ws.begin(), ws.end());
}

std::vector<LanInfo> get_mac_addresses() {
    std::vector<LanInfo> result;
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;

    DWORD dwRetVal = GetAdaptersAddresses(
        AF_UNSPEC,
        GAA_FLAG_INCLUDE_PREFIX,
        nullptr,
        nullptr,
        &outBufLen
    );

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (!pAddresses) return result;

        dwRetVal = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_INCLUDE_PREFIX,
            nullptr,
            pAddresses,
            &outBufLen
        );
    } else {
        return result;
    }

    if (dwRetVal != ERROR_SUCCESS) {
        free(pAddresses);
        return result;
    }

    for (PIP_ADAPTER_ADDRESSES pAdapter = pAddresses;
         pAdapter != nullptr;
         pAdapter = pAdapter->Next) {

        if (pAdapter->PhysicalAddressLength == 0 ||
            pAdapter->PhysicalAddressLength > 8)
            continue;

        LanInfo info;

        info.name = wchar_to_string(pAdapter->FriendlyName);
        info.interface_name = wchar_to_string(pAdapter->Description);

        char macStr[32] = {0};
        for (UINT i = 0; i < pAdapter->PhysicalAddressLength; ++i) {
            if (i > 0) sprintf(macStr + strlen(macStr), "-");
            sprintf(macStr + strlen(macStr), "%02X",
                    pAdapter->PhysicalAddress[i]);
        }
        info.mac = macStr;

        // Ищем первый IPv4
        info.ipv4 = "";
        for (PIP_ADAPTER_UNICAST_ADDRESS ua = pAdapter->FirstUnicastAddress;
             ua != nullptr;
             ua = ua->Next) {
            if (ua->Address.lpSockaddr->sa_family == AF_INET) {
                info.ipv4 = sockaddr_to_ipv4(ua->Address.lpSockaddr);
                break;
            }
        }

        info.is_active = (pAdapter->OperStatus == IfOperStatusUp);

        result.push_back(std::move(info));
    }

    free(pAddresses);
    return result;
}

int main() {
    auto macs = get_mac_addresses();
    for (const auto& m : macs) {
        std::cout << "Interface:      " << m.interface_name << "\n"
                  << "Adapter (name): " << m.name << "\n"
                  << "MAC:            " << m.mac << "\n"
                  << "IPv4:           " << (m.ipv4.empty() ? "(none)" : m.ipv4) << "\n"
                  << "Active:         " << (m.is_active ? "Yes" : "No") << "\n\n";
    }
    return 0;
}
