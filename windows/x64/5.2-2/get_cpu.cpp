// <- get_cpu.cpp
#include "main.h"
#include <windows.h>
#include <wbemidl.h>     // WMI
#include <comdef.h>      // Оставляем только для VARIANT, но НЕ используем _bstr_t
#include <string>
#include <vector>

// Не нужно: #pragma comment(lib, "wbemuuid.lib") — это только для MSVC

bool TInventory::get_cpu() {
    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes)) {
        id_cpu = "Не удалось инициализировать COM";
        return false;
    }

    // Безопасность COM (игнорируем RPC_E_ACCESS_DENIED — это нормально для локальных запросов)
    hRes = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                RPC_C_AUTHN_LEVEL_DEFAULT,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                nullptr, EOAC_NONE, nullptr);
    if (FAILED(hRes) && hRes != RPC_E_ACCESS_DENIED) {
        CoUninitialize();
        id_cpu = "Ошибка безопасности COM";
        return false;
    }

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        CoUninitialize();
        id_cpu = "Не удалось создать WbemLocator";
        return false;
    }

    IWbemServices* pSvc = nullptr;
    // Используем BSTR вручную вместо _bstr_t
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    if (!bstrNamespace) {
        pLoc->Release();
        CoUninitialize();
        id_cpu = "Не удалось выделить BSTR для namespace";
        return false;
    }

    hRes = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                               WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hRes) || !pSvc) {
        CoUninitialize();
        id_cpu = "Не удалось подключиться к WMI";
        return false;
    }

    hRes = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                             nullptr, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hRes)) {
        pSvc->Release();
        CoUninitialize();
        id_cpu = "Ошибка прокси-бланкета WMI";
        return false;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrQuery = SysAllocString(L"SELECT Name FROM Win32_Processor");
    BSTR bstrWQL = SysAllocString(L"WQL");
    if (!bstrQuery || !bstrWQL) {
        pSvc->Release();
        CoUninitialize();
        id_cpu = "Не удалось выделить BSTR для запроса";
        return false;
    }

    hRes = pSvc->ExecQuery(bstrWQL, bstrQuery,
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
    SysFreeString(bstrQuery);
    SysFreeString(bstrWQL);
    pSvc->Release();

    if (FAILED(hRes) || !pEnumerator) {
        CoUninitialize();
        id_cpu = "Запрос WMI не выполнен";
        return false;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    std::vector<std::string> cpuNames;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        VariantInit(&vtProp);

        HRESULT getRes = pclsObj->Get(L"Name", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(getRes) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
            // Конвертируем BSTR (UTF-16) -> UTF-8 std::string
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                std::string utf8(size, 0);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, &utf8[0], size, nullptr, nullptr);
                cpuNames.push_back(utf8);
            }
        }
        VariantClear(&vtProp);
        pclsObj->Release();
        pclsObj = nullptr;
    }
    pEnumerator->Release();

    CoUninitialize();

    if (cpuNames.empty()) {
        id_cpu = "Не найдено ни одного процессора";
        return false;
    }

    // Собираем все процессоры через запятую
    std::string result;
    for (size_t i = 0; i < cpuNames.size(); ++i) {
        if (i > 0) result += ", ";
        result += cpuNames[i];
    }

    id_cpu = std::move(result);

    // --- Лёгкая очистка строки (как раньше) ---
    const std::string patterns[] = {
        "(R)", "(TM)",
        "Core(TM) ",
        " Dual Core Processor ",
        " Quad Core Processor ",
        "@",
        "CPU @ ",
        "Processor"
    };

    for (const auto& pat : patterns) {
        size_t pos = 0;
        while ((pos = id_cpu.find(pat, pos)) != std::string::npos) {
            id_cpu.erase(pos, pat.length());
        }
    }

    auto normalize_spaces = [](std::string& s) {
        if (s.empty()) return;
        std::string res;
        res.reserve(s.size());
        bool lastWasSpace = true;
        for (char c : s) {
            if (c == ' ') {
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
        s = std::move(res);
    };
    normalize_spaces(id_cpu);

    return !id_cpu.empty();
}
