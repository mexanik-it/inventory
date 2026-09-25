// get_vga.cpp
#include <winsock2.h>
#include <windows.h>
#include <wbemidl.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <utility>

#include "main.h"

/***************************************************************************************************/
/* Конвертация wstring → UTF-8 string (перенесена наверх, т.к. нужна в get_gpus)                     */
/***************************************************************************************************/
static std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
                                   static_cast<int>(wstr.size()),
                                   nullptr, 0, nullptr, nullptr);
    if (size <= 0) return "";
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
                        static_cast<int>(wstr.size()),
                        &result[0], size, nullptr, nullptr);
    return result;
}

/***************************************************************************************************/
/* Получение списка видеоконтроллеров через WMI                                                     */
/***************************************************************************************************/
std::vector<GpuInfo> get_gpus() {
    std::vector<GpuInfo> result;

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cerr << "[get_vga] CoInitializeEx failed: 0x"
                  << std::hex << hr << std::dec << "\n";
        return result;
    }
    bool bWeInit = SUCCEEDED(hr);

    if (bWeInit) {
        hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                  RPC_C_AUTHN_LEVEL_DEFAULT,
                                  RPC_C_IMP_LEVEL_IMPERSONATE,
                                  nullptr, EOAC_NONE, nullptr);
        if (FAILED(hr) && hr != RPC_E_ACCESS_DENIED && hr != RPC_E_TOO_LATE) {
            std::cerr << "[get_vga] CoInitializeSecurity failed: 0x"
                      << std::hex << hr << std::dec << "\n";
            if (bWeInit) CoUninitialize();
            return result;
        }
    }

    IWbemLocator* pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hr) || !pLoc) {
        std::cerr << "[get_vga] CoCreateInstance failed: 0x"
                  << std::hex << hr << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return result;
    }

    IWbemServices* pSvc = nullptr;
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    hr = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                             WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hr) || !pSvc) {
        std::cerr << "[get_vga] ConnectServer failed: 0x"
                  << std::hex << hr << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return result;
    }

    hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                           nullptr, RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hr)) {
        std::cerr << "[get_vga] CoSetProxyBlanket failed: 0x"
                  << std::hex << hr << std::dec << "\n";
        pSvc->Release();
        if (bWeInit) CoUninitialize();
        return result;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrWQL = SysAllocString(L"WQL");
    BSTR bstrQuery = SysAllocString(
        L"SELECT Name, AdapterRAM FROM Win32_VideoController");
    hr = pSvc->ExecQuery(bstrWQL, bstrQuery,
                         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                         nullptr, &pEnumerator);
    SysFreeString(bstrQuery);
    SysFreeString(bstrWQL);
    pSvc->Release();

    if (FAILED(hr) || !pEnumerator) {
        std::cerr << "[get_vga] ExecQuery failed: 0x"
                  << std::hex << hr << std::dec << "\n";
        if (bWeInit) CoUninitialize();
        return result;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        GpuInfo info{};

        VARIANT vtProp;
        VariantInit(&vtProp);

        // Name
        HRESULT getRes = pclsObj->Get(L"Name", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(getRes) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            info.name = vtProp.bstrVal;
        } else {
            std::cerr << "[get_vga] Get(Name) failed, hr=0x"
                      << std::hex << getRes << " vt=" << vtProp.vt << "\n";
            VariantClear(&vtProp);
            pclsObj->Release();
            pclsObj = nullptr;
            continue;
        }
        VariantClear(&vtProp);

        // AdapterRAM
        VariantInit(&vtProp);
        getRes = pclsObj->Get(L"AdapterRAM", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(getRes)) {
            if (vtProp.vt == VT_UI8) {
                info.memory_bytes = vtProp.ullVal;
            } else if (vtProp.vt == VT_I4) {
                info.memory_bytes = static_cast<uint64_t>(vtProp.lVal);
            } else if (vtProp.vt == VT_UINT) {
                info.memory_bytes = static_cast<uint64_t>(vtProp.uintVal);
            } else if (vtProp.vt == VT_NULL || vtProp.vt == VT_EMPTY) {
                info.memory_bytes = 0;
            } else {
                std::cerr << "[get_vga] AdapterRAM unexpected vt="
                          << vtProp.vt << "\n";
                info.memory_bytes = 0;
            }
        } else {
            std::cerr << "[get_vga] Get(AdapterRAM) failed, hr=0x"
                      << std::hex << getRes << "\n";
            info.memory_bytes = 0;
        }
        VariantClear(&vtProp);

        // Лог с конвертацией wstring → string
        std::cerr << "[get_vga] Found GPU: '" << wstringToUtf8(info.name)
                  << "' RAM=" << info.memory_bytes << " bytes\n";

        result.push_back(std::move(info));
        pclsObj->Release();
        pclsObj = nullptr;
    }

    pEnumerator->Release();
    if (bWeInit) CoUninitialize();

    std::cerr << "[get_vga] Total GPUs found: " << result.size() << "\n";
    return result;
}

/***************************************************************************************************/
/* TInventory::get_vga                                                                              */
/***************************************************************************************************/
bool TInventory::get_vga() {
    auto gpus = get_gpus();

    id_vga.clear();
    id_vga_size.clear();

    if (gpus.empty()) {
        id_vga      = "unknown";
        id_vga_size = "unknown";
        std::cerr << "[get_vga] No GPUs found, setting unknown\n";
        return true;
    }

    for (size_t i = 0; i < gpus.size(); ++i) {
        const auto& gpu = gpus[i];

        if (i > 0) {
            id_vga      += " / ";
            id_vga_size += " / ";
        }

        id_vga += wstringToUtf8(gpu.name);

        if (gpu.memory_bytes > 0) {
            uint64_t gb = (gpu.memory_bytes + 536870912ULL) / 1073741824ULL;
            id_vga_size += std::to_string(gb) + "Gb";
        } else {
            id_vga_size += "unknown";
        }
    }
/*
    std::cerr << "[get_vga] id_vga='" << id_vga
              << "' id_vga_size='" << id_vga_size << "'\n";
*/
    return true;
}
