#include <windows.h>
#include <wbemidl.h>
#include <iostream>
#include <string>

struct MbInfo {
    std::string manufacturer;
    std::string product;
    std::string uuid;
};

// ── Внутренний хелпер: выполнение WQL-запроса ─────────────────────────
static bool wmi_query(IWbemServices *pSvc, const wchar_t *query,
                      IWbemClassObject **pclsObj) {
    IEnumWbemClassObject *pEnum = nullptr;
    BSTR bstrLang = SysAllocString(L"WQL");
    BSTR bstrQ   = SysAllocString(query);
    HRESULT hr = pSvc->ExecQuery(bstrLang, bstrQ,
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        nullptr, &pEnum);
    SysFreeString(bstrLang);
    SysFreeString(bstrQ);
    if (FAILED(hr) || !pEnum) return false;

    ULONG ret = 0;
    bool ok = (pEnum->Next(WBEM_INFINITE, 1, pclsObj, &ret) == S_OK && ret == 1);
    pEnum->Release();
    return ok;
}

// ── Внутренний хелпер: достать строковое свойство ─────────────────────
static std::string wmi_get_string(IWbemClassObject *obj, const wchar_t *prop) {
    VARIANT vt;
    VariantInit(&vt);
    std::string result;
    if (SUCCEEDED(obj->Get(prop, 0, &vt, 0, 0)) && vt.vt == VT_BSTR) {
        std::wstring w(vt.bstrVal);
        result = std::string(w.begin(), w.end());
    }
    VariantClear(&vt);
    return result;
}

// ── Главная обёртка ───────────────────────────────────────────────────
MbInfo get_mb_info() {
    MbInfo info{};

    if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
        return info;

    CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr);

    // Подключение к WMI
    IWbemLocator *pLoc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                IID_IWbemLocator,
                                reinterpret_cast<LPVOID*>(&pLoc)))) {
        CoUninitialize();
        return info;
    }

    IWbemServices *pSvc = nullptr;
    BSTR bstrNs = SysAllocString(L"ROOT\\CIMV2");
    pLoc->ConnectServer(bstrNs, nullptr, nullptr, nullptr,
                        0, nullptr, nullptr, &pSvc);
    SysFreeString(bstrNs);
    pLoc->Release();

    if (!pSvc) {
        CoUninitialize();
        return info;
    }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                      nullptr, RPC_C_AUTHN_LEVEL_CALL,
                      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

    // Запрос 1: Manufacturer + Product (материнская плата)
    IWbemClassObject *obj = nullptr;
    if (wmi_query(pSvc,
        L"SELECT Manufacturer, Product FROM Win32_BaseBoard", &obj)) {
        info.manufacturer = wmi_get_string(obj, L"Manufacturer");
        info.product      = wmi_get_string(obj, L"Product");
        obj->Release();
    }

    // Запрос 2: UUID (системный продукт)
    if (wmi_query(pSvc,
        L"SELECT UUID FROM Win32_ComputerSystemProduct", &obj)) {
        info.uuid = wmi_get_string(obj, L"UUID");
        obj->Release();
    }

    pSvc->Release();
    CoUninitialize();
    return info;
}

// ── Точка входа ───────────────────────────────────────────────────────
int main() {
    MbInfo info = get_mb_info();
    std::cout << "Manufacturer: " << info.manufacturer << "\n";
    std::cout << "Product:      " << info.product      << "\n";
    std::cout << "UUID:         " << info.uuid         << "\n";
    return 0;
}
