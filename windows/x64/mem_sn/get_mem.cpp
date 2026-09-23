#include <windows.h>
#include <wbemidl.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")

struct MemInfo {
    std::wstring manufacturer;
    std::wstring partNumber;
    std::wstring serialNumber;
    uint64_t capacityBytes;
    unsigned long speedMhz;      // 0, если не известно
    unsigned short formFactor;   // см. Win32_PhysicalMemory.FormFactor
};

std::vector<MemInfo> get_memory() {
    std::vector<MemInfo> result;

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
        SysAllocString(L"SELECT Manufacturer,PartNumber,SerialNumber,Capacity,Speed,FormFactor FROM Win32_PhysicalMemory"),
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

        MemInfo info{};
        info.capacityBytes = 0;
        info.speedMhz = 0;
        info.formFactor = 0;

        HRESULT hr;

        hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) info.manufacturer = vtProp.bstrVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"PartNumber", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) info.partNumber = vtProp.bstrVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) info.serialNumber = vtProp.bstrVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Capacity", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr)) {
            if (vtProp.vt == VT_UI8) info.capacityBytes = vtProp.ullVal;
            else if (vtProp.vt == VT_I4) info.capacityBytes = static_cast<uint64_t>(vtProp.lVal);
        }
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Speed", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)) {
            info.speedMhz = static_cast<unsigned long>(vtProp.ulVal);
        }
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"FormFactor", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hr) && (vtProp.vt == VT_I2 || vtProp.vt == VT_UI2)) {
            info.formFactor = static_cast<unsigned short>(vtProp.uiVal);
        }
        VariantClear(&vtProp);

        // Добавляем, даже если часть полей пуста (например, SerialNumber)
        result.push_back(info);

        pclsObj->Release();
    }
    pEnumerator->Release();
    CoUninitialize();
    return result;
}

int main() {
    auto mem = get_memory();
    for (size_t i = 0; i < mem.size(); ++i) {
        const auto& m = mem[i];
        std::wcout << L"Memory module #" << (i + 1) << L"\n";
        if (!m.manufacturer.empty())
            std::wcout << L"  Manufacturer: " << m.manufacturer << L"\n";
        if (!m.partNumber.empty())
            std::wcout << L"  PartNumber: " << m.partNumber << L"\n";
        if (!m.serialNumber.empty())
            std::wcout << L"  Serial: " << m.serialNumber << L"\n";
        std::wcout << L"  Capacity: " << (m.capacityBytes / (1024ULL * 1024 * 1024)) << L" GB\n";
        if (m.speedMhz > 0)
            std::wcout << L"  Speed: " << m.speedMhz << L" MHz\n";
        std::wcout << L"\n";
    }
    return 0;
}

