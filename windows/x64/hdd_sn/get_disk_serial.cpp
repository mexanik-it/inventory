#include <windows.h>
#include <wbemidl.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")

struct DiskInfo {
    std::wstring model;
    std::wstring serialNumber;
};

std::vector<DiskInfo> get_disks() {
    std::vector<DiskInfo> result;

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
        SysAllocString(L"SELECT Model, SerialNumber FROM Win32_DiskDrive"),
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

        DiskInfo info;

        HRESULT hrModel = pclsObj->Get(L"Model", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hrModel) && vtProp.vt == VT_BSTR) {
            info.model = vtProp.bstrVal;
        }
        VariantClear(&vtProp);

        VariantInit(&vtProp);
        HRESULT hrSerial = pclsObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hrSerial) && vtProp.vt == VT_BSTR) {
            info.serialNumber = vtProp.bstrVal;
        }
        VariantClear(&vtProp);

        if (!info.model.empty() || !info.serialNumber.empty()) {
            result.push_back(info);
        }

        pclsObj->Release();
    }
    pEnumerator->Release();
    CoUninitialize();
    return result;
}

int main() {
    auto disks = get_disks();
    for (size_t i = 0; i < disks.size(); ++i) {
        const auto& d = disks[i];
        std::wcout << L"Disk #" << (i + 1) << L"\n";
        std::wcout << L"  Model: " << d.model << L"\n";
        std::wcout << L"  Serial: " << d.serialNumber << L"\n\n";
    }
    return 0;
}
