#include "main.h"
#include <cmath>
#include <iostream>
#include <windows.h>

/***************************************************************************************************/
/* возвращает общее количество оперативной памяти в Gb                                             */
/* а так-же названия модулей и их размер в Gb                                                      */ 
/***************************************************************************************************/

#include <windows.h>
#include <string>
#include <cmath>
#include <iostream>
#include <wbemidl.h>

std::vector<MemoryModule> getMemoryModules() {
    std::vector<MemoryModule> result;

    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes)) return result;

    hRes = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                nullptr, EOAC_NONE, nullptr);
    if (FAILED(hRes) && hRes != RPC_E_ACCESS_DENIED) {
        CoUninitialize();
        return result;
    }

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        CoUninitialize();
        return result;
    }

    IWbemServices* pSvc = nullptr;
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    if (!bstrNamespace) {
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    hRes = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                               WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hRes) || !pSvc) {
        CoUninitialize();
        return result;
    }

    hRes = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                             nullptr, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hRes)) {
        pSvc->Release();
        CoUninitialize();
        return result;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrQuery = SysAllocString(L"SELECT Manufacturer, PartNumber, SerialNumber, Capacity FROM Win32_PhysicalMemory");
    BSTR bstrWQL = SysAllocString(L"WQL");
    if (!bstrQuery || !bstrWQL) {
        pSvc->Release();
        CoUninitialize();
        return result;
    }

    hRes = pSvc->ExecQuery(bstrWQL, bstrQuery,
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
    SysFreeString(bstrQuery);
    SysFreeString(bstrWQL);
    pSvc->Release();

    if (FAILED(hRes) || !pEnumerator) {
        CoUninitialize();
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
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                mod.serialNumber.resize(size - 1);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, &mod.serialNumber[0], size, nullptr, nullptr);
            }
        }
        VariantClear(&vtProp);

        // Capacity — может приходить как VT_UI8 или VT_BSTR (строка с числом)
        VariantInit(&vtProp);
        hr = pclsObj->Get(L"Capacity", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr)) {
            if (vtProp.vt == VT_UI8) {
                mod.capacityBytes = vtProp.ullVal;
            } else if (vtProp.vt == VT_BSTR && vtProp.bstrVal) {
                mod.capacityBytes = _wcstoui64(vtProp.bstrVal, nullptr, 10);
            }
        }
        VariantClear(&vtProp);

        result.push_back(mod);
        pclsObj->Release();
        pclsObj = nullptr;
    }

    pEnumerator->Release();
    CoUninitialize();
    return result;
}

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
    if (modules.empty()) {
        id_mem_name = "unknown";
        return true;
    }

    id_mem_name.clear();  // ← ВАЖНО: очищаем перед заполнением
    for (size_t i = 0; i < modules.size(); ++i) {
        if (i > 0) id_mem_name += " / ";

        std::string capStr;
        if (modules[i].capacityBytes > 0) {
            double gb = static_cast<double>(modules[i].capacityBytes)
                      / (1024.0 * 1024.0 * 1024.0);
            capStr = std::to_string(static_cast<long long>(gb)) + "Gb";
        } else {
            capStr = "unknown";
        }

        id_mem_name += (modules[i].manufacturer.empty() ? "Unknown" : modules[i].manufacturer)
                     + " "
                     + (modules[i].partNumber.empty() ? "" : modules[i].partNumber)
                     + " (" + capStr + ")";
    }

    // НЕ перезаписываем id_mem — он уже содержит "8Gb"
    return true;
}
