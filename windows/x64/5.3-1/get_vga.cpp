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

// #pragma comment и comdef.h убраны — MinGW их не поддерживает

// 1. Инициализация COM
HRESULT init_com() {
    return CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

// 2. Подключение к WMI (ROOT\CIMV2)
HRESULT connect_to_wmi(IWbemLocator*& pLoc, IWbemServices*& pSvc) {
    HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hr) || !pLoc) return hr;

    BSTR bstrRoot = SysAllocString(L"ROOT\\CIMV2");
    if (!bstrRoot) return E_OUTOFMEMORY;

    hr = pLoc->ConnectServer(bstrRoot, nullptr, nullptr, nullptr,
                             0, 0, nullptr, &pSvc);
    SysFreeString(bstrRoot);
    if (FAILED(hr) || !pSvc) return hr;

    hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                           RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                           nullptr, EOAC_NONE);
    return hr;
}

// 3. Выполнение запроса и получение перечислителя
HRESULT query_video_controllers(IWbemServices* pSvc, IEnumWbemClassObject*& pEnumerator) {
    BSTR bstrWQL = SysAllocString(L"WQL");
    BSTR bstrQuery = SysAllocString(L"SELECT Name, AdapterRAM FROM Win32_VideoController");
    if (!bstrWQL || !bstrQuery) {
        SysFreeString(bstrWQL);
        SysFreeString(bstrQuery);
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pSvc->ExecQuery(bstrWQL, bstrQuery,
                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 nullptr, &pEnumerator);
    SysFreeString(bstrWQL);
    SysFreeString(bstrQuery);
    return hr;
}

// 4. Извлечение данных из одного объекта (Name, AdapterRAM)
bool extract_gpu_info(IWbemClassObject* pObj, GpuInfo& out) {
    VARIANT vtProp;
    VariantInit(&vtProp);

    // Name
    HRESULT hr = pObj->Get(L"Name", 0, &vtProp, nullptr, nullptr);
    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
        out.name = vtProp.bstrVal ? vtProp.bstrVal : L"";
    } else {
        VariantClear(&vtProp);
        return false;
    }
    VariantClear(&vtProp);

    // AdapterRAM
    VariantInit(&vtProp);
    hr = pObj->Get(L"AdapterRAM", 0, &vtProp, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        if (vtProp.vt == VT_UI8) {
            out.memory_bytes = vtProp.ullVal;
        } else if (vtProp.vt == VT_I4) {
            out.memory_bytes = static_cast<uint64_t>(vtProp.lVal);
        } else if (vtProp.vt == VT_NULL || vtProp.vt == VT_EMPTY) {
            out.memory_bytes = 0;
        } else {
            VariantClear(&vtProp);
            return false;
        }
    } else {
        VariantClear(&vtProp);
        return false;
    }
    VariantClear(&vtProp);
    return true;
}

// 5. Форматирование памяти: байты → ГБ
double format_vram_gb(uint64_t bytes) {
    return static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
}

uint64_t format_vram_gb_int(uint64_t bytes) {
    return (bytes + 536870912ULL) / 1073741824ULL;
}

// 6. Главная функция: собирает всё и возвращает вектор
std::vector<GpuInfo> get_gpus() {
    std::vector<GpuInfo> result;

    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;
    IWbemClassObject* pObj = nullptr;
    ULONG uReturn = 0;

    HRESULT hr = init_com();
    if (FAILED(hr)) return result;

    hr = connect_to_wmi(pLoc, pSvc);
    if (FAILED(hr)) {
        CoUninitialize();
        return result;
    }

    hr = query_video_controllers(pSvc, pEnumerator);
    if (FAILED(hr) || !pEnumerator) {
        if (pSvc) pSvc->Release();
        if (pLoc) pLoc->Release();
        CoUninitialize();
        return result;
    }

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn == 1) {
        GpuInfo info;
        if (extract_gpu_info(pObj, info)) {
            result.push_back(std::move(info));
        }
        pObj->Release();
        pObj = nullptr;
        uReturn = 0;
    }

    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();
    return result;
}

bool TInventory::get_vga() {
    auto gpus = get_gpus();
    for (const auto& gpu : gpus) {
        // wstring → string через системную кодировку (CP_ACP)
        std::string name_str;
        int len = WideCharToMultiByte(CP_ACP, 0, gpu.name.c_str(),
                                     static_cast<int>(gpu.name.size()),
                                     nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            name_str.resize(len);
            WideCharToMultiByte(CP_ACP, 0, gpu.name.c_str(),
                                static_cast<int>(gpu.name.size()),
                                &name_str[0], len, nullptr, nullptr);
        }

        id_vga      = name_str;
        id_vga_size = std::to_string(format_vram_gb_int(gpu.memory_bytes)) + " GB";
    }
    return true;
}
