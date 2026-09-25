#include "main.h"
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <wbemidl.h>
#include <comdef.h>

/***************************************************************************************************/
/* WMI-запрос для материнской платы                                                                 */
/***************************************************************************************************/
static bool wmiQuerySingle(const std::wstring& wql,
                           const std::wstring& property,
                           std::string& outResult) {
    outResult.clear();

    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes) && hRes != RPC_E_CHANGED_MODE) {
        std::cerr << "[get_mb] CoInitializeEx failed: 0x"
                  << std::hex << hRes << std::dec << "\n";
        return false;
    }
    bool bWeInit = SUCCEEDED(hRes);

    if (bWeInit) {
        hRes = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                     RPC_C_AUTHN_LEVEL_DEFAULT,
                                     RPC_C_IMP_LEVEL_IMPERSONATE,
                                     nullptr, EOAC_NONE, nullptr);
        if (FAILED(hRes) && hRes != RPC_E_ACCESS_DENIED) {
            std::cerr << "[get_mb] CoInitializeSecurity failed: 0x"
                      << std::hex << hRes << std::dec << "\n";
            if (bWeInit) CoUninitialize();
            return false;
        }
    }

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        std::cerr << "[get_mb] CoCreateInstance failed: 0x"
                  << std::hex << hRes << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return false;
    }

    IWbemServices* pSvc = nullptr;
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    hRes = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                               WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hRes) || !pSvc) {
        std::cerr << "[get_mb] ConnectServer failed: 0x"
                  << std::hex << hRes << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return false;
    }

    hRes = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                             nullptr, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hRes)) {
        std::cerr << "[get_mb] CoSetProxyBlanket failed: 0x"
                  << std::hex << hRes << std::dec << "\n";
        pSvc->Release();
        if (bWeInit) CoUninitialize();
        return false;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrWQL = SysAllocString(L"WQL");
    BSTR bstrQuery = SysAllocString(wql.c_str());
    hRes = pSvc->ExecQuery(bstrWQL, bstrQuery,
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
    SysFreeString(bstrQuery);
    SysFreeString(bstrWQL);
    pSvc->Release();

    if (FAILED(hRes) || !pEnumerator) {
        std::cerr << "[get_mb] ExecQuery failed: 0x"
                  << std::hex << hRes << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return false;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    bool found = false;
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        VariantInit(&vtProp);
        HRESULT hr = pclsObj->Get(property.c_str(), 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1,
                                           nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                outResult.resize(size - 1);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1,
                                    &outResult[0], size, nullptr, nullptr);
                found = true;
            }
        }
        VariantClear(&vtProp);
        pclsObj->Release();
        pclsObj = nullptr;

        if (found) break;  // берём первую запись
    }

    pEnumerator->Release();
    if (bWeInit) CoUninitialize();
    return found;
}

/***************************************************************************************************/
/* trim: убрать пробелы и \r\n по краям                                                             */
/***************************************************************************************************/
static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    size_t last  = s.find_last_not_of(" \t\n\r");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return s.substr(first, last - first + 1);
}

/***************************************************************************************************/
/* TInventory::get_mb                                                                               */
/***************************************************************************************************/
bool TInventory::get_mb() {
    std::string manufacturer, product, serial;

    wmiQuerySingle(L"SELECT Manufacturer FROM Win32_BaseBoard",
                   L"Manufacturer", manufacturer);
    wmiQuerySingle(L"SELECT Product FROM Win32_BaseBoard",
                   L"Product", product);
    wmiQuerySingle(L"SELECT SerialNumber FROM Win32_BaseBoard",
                   L"SerialNumber", serial);

    manufacturer = trim(manufacturer);
    product      = trim(product);
    serial       = trim(serial);

    // Убираем суффиксы из Manufacturer
    auto removeSuffix = [](std::string& s, const std::string& suffix) {
        size_t pos = s.find(suffix);
        if (pos != std::string::npos)
            s.erase(pos, suffix.length());
    };

    removeSuffix(manufacturer, "COMPUTER INC.  ");
    removeSuffix(manufacturer, "Technology Co., Ltd.  ");
    removeSuffix(manufacturer, "Technology Co., Ltd.");

    id_mb.clear();
    if (!manufacturer.empty()) {
        id_mb = manufacturer;
    }
    if (!product.empty()) {
        if (!id_mb.empty()) id_mb += " ";
        id_mb += product;
    }
    if (id_mb.empty()) {
        id_mb = "unknown";
    }

    sn_mb = serial.empty() ? "unknown" : serial;
/*
    std::cerr << "[get_mb] Manufacturer='" << manufacturer
              << "' Product='" << product
              << "' Serial='" << sn_mb << "'\n";
*/
    return true;
}
