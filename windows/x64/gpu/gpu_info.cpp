#include <windows.h>
#include <wbemidl.h>
#include <iostream>
#include <string>
#include <vector>

struct GpuInfo {
    std::wstring name;
    uint64_t vram_bytes;
};

std::vector<GpuInfo> get_gpus() {
    std::vector<GpuInfo> result;

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return result;

    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hres) || !pLoc) {
        CoUninitialize();
        return result;
    }

    IWbemServices* pSvc = nullptr;
    hres = pLoc->ConnectServer(
        SysAllocString(L"ROOT\\CIMV2"), nullptr, nullptr,
        nullptr, 0, nullptr, nullptr, &pSvc);
    pLoc->Release();
    if (FAILED(hres)) {
        CoUninitialize();
        return result;
    }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                      nullptr, RPC_C_AUTHN_LEVEL_CALL,
                      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = nullptr;
    hres = pSvc->ExecQuery(
        SysAllocString(L"WQL"),
        SysAllocString(L"SELECT Name, AdapterRAM FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &pEnumerator);
    pSvc->Release();
    if (FAILED(hres) || !pEnumerator) {
        CoUninitialize();
        return result;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn == 1) {
        VARIANT vtProp;
        VariantInit(&vtProp);

        std::wstring name;
        HRESULT hrName = pclsObj->Get(L"Name", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hrName) && vtProp.vt == VT_BSTR) {
            name = vtProp.bstrVal;
        }
        VariantClear(&vtProp);

        uint64_t ram = 0;
        VariantInit(&vtProp);
        HRESULT hrRam = pclsObj->Get(L"AdapterRAM", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hrRam)) {
            if (vtProp.vt == VT_I4)
                ram = static_cast<uint64_t>(vtProp.lVal);
            else if (vtProp.vt == VT_UI4)
                ram = static_cast<uint64_t>(vtProp.ulVal);
            else if (vtProp.vt == VT_I8)
                ram = static_cast<uint64_t>(vtProp.llVal);
            else if (vtProp.vt == VT_UI8)
                ram = vtProp.ullVal;
        }
        VariantClear(&vtProp);

        if (!name.empty()) {
            GpuInfo info;
            info.name = name;
            info.vram_bytes = ram;
            result.push_back(info);
        }

        pclsObj->Release();
    }
    pEnumerator->Release();
    CoUninitialize();
    return result;
}

int main() {
    auto gpus = get_gpus();
    for (size_t i = 0; i < gpus.size(); ++i) {
        const auto& gpu = gpus[i];
        std::wcout << L"GPU: " << gpu.name << L"\n";
        std::wcout << L"VRAM: " << (gpu.vram_bytes / (1024 * 1024)) << L" MB\n\n";
    }
    return 0;
}
