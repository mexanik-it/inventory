#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "main.h"

//#pragma comment(lib, "ws2_32.lib")

static std::string sockaddr_to_ipv4(const SOCKADDR *addr) {
    if (!addr || addr->sa_family != AF_INET)
        return "";
    const sockaddr_in *sin = reinterpret_cast<const sockaddr_in*>(addr);
    return inet_ntoa(sin->sin_addr);
}

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

bool TInventory::get_lan() {
    auto macs = get_mac_addresses();
    id_mac.clear();
    id_ip.clear();

    if (macs.empty()) {
        id_mac = "Unknown";
        id_ip = "Unknown";
        return true;
    }

    for (size_t i = 0; i < macs.size(); ++i) {
        const auto& m = macs[i];

        id_mac += (!m.mac.empty() ? m.mac : "NoMAC");
        id_ip  += (!m.ipv4.empty() ? m.ipv4 : "(none)");

        if (i + 1 < macs.size()) {
            id_mac += " / ";
            id_ip  += " / ";
        }
    }

    return true;
}
