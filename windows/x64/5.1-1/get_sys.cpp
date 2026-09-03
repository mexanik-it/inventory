#include "main.h"
#include <windows.h>
#include <winnt.h>
#include <string>
#include <cstdio>

static std::string product_type_to_name(DWORD productType) {
    switch (productType) {
        // Явно добавляем 191 (0xBF) как IoT Enterprise LTSC
        case 191:
        case 0x000000B3: return "Enterprise LTSC";
        case 0x000000B4: return "Enterprise LTSC";

        case 0x00000067: return "Home";
        case 0x0000006A: return "Pro N";
        case 0x0000006F: return "Single Language";
        case 0x00000070: return "Country Specific";
        case 0x0000007D: return "Enterprise LTSC";
        case 0x0000007E: return "Enterprise LTSC N";
        case 0x00000082: return "Cloud";
        case 0x00000083: return "Cloud N";
        case 0x000000B5: return "IoT UAP Container";

        // Оставшиеся PRODUCT_* из winnt.h
        case PRODUCT_HOME_BASIC:      return "Home Basic";
        case PRODUCT_HOME_PREMIUM:    return "Home Premium";
        case PRODUCT_PROFESSIONAL:    return "Pro";
        case PRODUCT_ENTERPRISE:     return "Enterprise";
        case PRODUCT_ULTIMATE:        return "Ultimate";
        case PRODUCT_CORE:            return "";
        case PRODUCT_EDUCATION:       return "Education";
        case PRODUCT_EDUCATION_N:      return "Education N";
        case PRODUCT_IOTUAP:          return "IoT UAP";

        default:
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Unknown (%lu)", static_cast<unsigned long>(productType));
            return buf;
    }
}

bool TInventory::get_sys() {
    OSVERSIONINFOEXA osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (!GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&osvi))) {
        id_sys = "Windows Unknown";
        return false;
    }

    const char* baseName = nullptr;

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
        baseName = "Windows Vista";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
        baseName = "Windows 7";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2) {
        baseName = "Windows 8";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3) {
        baseName = "Windows 8.1";
    } else if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0) {
        baseName = "Windows 10";
    } else {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Windows %lu.%lu",
                      static_cast<unsigned long>(osvi.dwMajorVersion),
                      static_cast<unsigned long>(osvi.dwMinorVersion));
        baseName = buf;
    }

    DWORD productType = 0;
    if (!GetProductInfo(osvi.dwMajorVersion, osvi.dwMinorVersion,
                        osvi.wServicePackMajor, osvi.wServicePackMinor, &productType)) {
        id_sys = baseName;
        return true;
    }

    std::string edition = product_type_to_name(productType);

    if (edition.empty()) {
        id_sys = baseName;                 // например, "Windows 8.1"
    } else {
        id_sys = std::string(baseName) + " " + edition;  // например, "Windows 10 IoT Enterprise LTSC"
    }

    return true;
}
