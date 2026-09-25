#include "main.h"
#include <cmath>
#include <iostream>
#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include <wbemidl.h>

/***************************************************************************************************/
/* Вспомогательные функции                                                                          */
/***************************************************************************************************/

static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

static std::string normalizeSpaces(const std::string& s) {
    std::string res;
    res.reserve(s.size());
    bool lastWasSpace = true;
    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!lastWasSpace) {
                res += ' ';
                lastWasSpace = true;
            }
        } else {
            res += c;
            lastWasSpace = false;
        }
    }
    if (!res.empty() && res.back() == ' ') res.pop_back();
    return res;
}

static bool isDummySerial(const std::string& s) {
    if (s.empty()) return true;

    char first = s[0];
    bool allSame = true;
    for (char c : s) {
        if (c != first) { allSame = false; break; }
    }
    if (allSame) return true;

    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "none" || lower == "null" || lower == "0" ||
        lower == "00000000" || lower == "ffffffff") {
        return true;
    }

    return false;
}

/***************************************************************************************************/
/* Получение модулей памяти через WMI                                                               */
/***************************************************************************************************/

std::vector<MemoryModule> getMemoryModules() {
    std::vector<MemoryModule> result;

    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes) && hRes != RPC_E_CHANGED_MODE) {
        return result;
    }
    bool bWeInit = SUCCEEDED(hRes);

    if (bWeInit) {
        hRes = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                    RPC_C_AUTHN_LEVEL_DEFAULT,
                                    RPC_C_IMP_LEVEL_IMPERSONATE,
                                    nullptr, EOAC_NONE, nullptr);
        if (FAILED(hRes) && hRes != RPC_E_ACCESS_DENIED) {
            CoUninitialize();
            return result;
        }
    }

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        if (bWeInit) CoUninitialize();
        return result;
    }

    IWbemServices* pSvc = nullptr;
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    if (!bstrNamespace) {
        pLoc->Release();
        if (bWeInit) CoUninitialize();
        return result;
    }

    hRes = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                               WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hRes) || !pSvc) {
        if (bWeInit) CoUninitialize();
        return result;
    }

    hRes = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                             nullptr, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hRes)) {
        pSvc->Release();
        if (bWeInit) CoUninitialize();
        return result;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrQuery = SysAllocString(
        L"SELECT Manufacturer, PartNumber, SerialNumber, Capacity FROM Win32_PhysicalMemory");
    BSTR bstrWQL = SysAllocString(L"WQL");
    if (!bstrQuery || !bstrWQL) {
        pSvc->Release();
        if (bWeInit) CoUninitialize();
        return result;
    }

    hRes = pSvc->ExecQuery(bstrWQL, bstrQuery,
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
    SysFreeString(bstrQuery);
    SysFreeString(bstrWQL);
    pSvc->Release();

    if (FAILED(hRes) || !pEnumerator) {
        if (bWeInit) CoUninitialize();
        return result;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        MemoryModule mod{};

        VARIANT vtProp;
        VariantInit(&vtProp);

        // Manufacturer
        HRESULT hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                mod.manufacturer.resize(size - 1);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, &mod.manufacturer[0], size, nullptr, nullptr);
            }
        }
        VariantClear(&vtProp);

        // PartNumber
        VariantInit(&vtProp);
        hr = pclsObj->Get(L"PartNumber", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                mod.partNumber.resize(size - 1);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, &mod.partNumber[0], size, nullptr, nullptr);
            }
        }
        VariantClear(&vtProp);

        // SerialNumber
        VariantInit(&vtProp);
        hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr)) {
            if (vtProp.vt == VT_BSTR && vtProp.bstrVal) {
                int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, nullptr, 0, nullptr, nullptr);
                if (size > 1) {
                    mod.serialNumber.resize(size - 1);
                    WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, &mod.serialNumber[0], size, nullptr, nullptr);
                }
            }
        }
        VariantClear(&vtProp);

        // Capacity
        VariantInit(&vtProp);
        hr = pclsObj->Get(L"Capacity", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr)) {
            if (vtProp.vt == VT_UI8) {
                mod.capacityBytes = vtProp.ullVal;
            } else if (vtProp.vt == VT_BSTR && vtProp.bstrVal) {
                wchar_t* endptr = nullptr;
                mod.capacityBytes = _wcstoui64(vtProp.bstrVal, &endptr, 10);
            }
        }
        VariantClear(&vtProp);

        result.push_back(mod);
        pclsObj->Release();
        pclsObj = nullptr;
    }

    pEnumerator->Release();
    if (bWeInit) CoUninitialize();
    return result;
}

/***************************************************************************************************/
/* Получение информации о памяти                                                                    */
/***************************************************************************************************/

bool TInventory::get_mem() {
    MEMORYSTATUSEX memoryInfo{};
    memoryInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memoryInfo)) {
        std::cerr << "Error getting information about memory: "
                  << GetLastError() << "\n";
        return false;
    }

    constexpr double bytesPerGiB = 1024.0 * 1024.0 * 1024.0;
    int intGB = static_cast<int>(std::round(
        static_cast<double>(memoryInfo.ullTotalPhys) / bytesPerGiB));
    id_mem = std::to_string(intGB) + "Gb";

    auto modules = getMemoryModules();

    id_mem_name.clear();
    sn_mem.clear();

    if (modules.empty()) {
        id_mem_name = "unknown";
        sn_mem = "unknown";
        return true;
    }

    for (size_t i = 0; i < modules.size(); ++i) {
        const auto& mod = modules[i];

        // --- id_mem_name: Manufacturer PartNumber (Capacity) ---
        if (i > 0) id_mem_name += " / ";

        std::string capStr;
        if (mod.capacityBytes > 0) {
            double gb = static_cast<double>(mod.capacityBytes) / bytesPerGiB;
            capStr = std::to_string(static_cast<long long>(gb)) + "Gb";
        } else {
            capStr = "unknown";
        }

        // Нормализуем пробелы: убираем лишние по краям и схлопываем множественные
        std::string man = mod.manufacturer.empty()
            ? "Unknown"
            : normalizeSpaces(mod.manufacturer);
        std::string pn  = normalizeSpaces(mod.partNumber);

        if (!pn.empty()) {
            id_mem_name += man + " " + pn + " (" + capStr + ")";
        } else {
            id_mem_name += man + " (" + capStr + ")";
        }

        // --- sn_mem: серийный номер модуля ---
        if (i > 0) sn_mem += " / ";

        std::string sn = trim(mod.serialNumber);

        if (isDummySerial(sn)) {
            sn = "unknown";
        }

        sn_mem += sn;
    }

    return true;
}
