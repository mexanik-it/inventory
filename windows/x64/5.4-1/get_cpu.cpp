// get_cpu.cpp
#include "main.h"
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>
#include <vector>
#include <iostream>

bool TInventory::get_cpu() {
    // --- COM init с обработкой RPC_E_CHANGED_MODE ---
    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes) && hRes != RPC_E_CHANGED_MODE) {
        id_cpu = "Не удалось инициализировать COM";
        return false;
    }
    bool bWeInit = SUCCEEDED(hRes);

    // CoInitializeSecurity: игнорируем RPC_E_ACCESS_DENIED и RPC_E_TOO_LATE
    // (security уже настроена предыдущим вызовом — это нормально)
    if (bWeInit) {
        hRes = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                    RPC_C_AUTHN_LEVEL_DEFAULT,
                                    RPC_C_IMP_LEVEL_IMPERSONATE,
                                    nullptr, EOAC_NONE, nullptr);
        if (FAILED(hRes) && hRes != RPC_E_ACCESS_DENIED && hRes != RPC_E_TOO_LATE) {
            CoUninitialize();
            id_cpu = "Ошибка безопасности COM";
            return false;
        }
    }

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        if (bWeInit) CoUninitialize();
        id_cpu = "Не удалось создать WbemLocator";
        return false;
    }

    IWbemServices* pSvc = nullptr;
    BSTR bstrNamespace = SysAllocString(L"ROOT\\CIMV2");
    if (!bstrNamespace) {
        pLoc->Release();
        if (bWeInit) CoUninitialize();
        id_cpu = "Не удалось выделить BSTR для namespace";
        return false;
    }

    hRes = pLoc->ConnectServer(bstrNamespace, 0, 0, 0,
                               WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, &pSvc);
    SysFreeString(bstrNamespace);
    pLoc->Release();

    if (FAILED(hRes) || !pSvc) {
        if (bWeInit) CoUninitialize();
        id_cpu = "Не удалось подключиться к WMI";
        return false;
    }

    hRes = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                             nullptr, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hRes)) {
        pSvc->Release();
        if (bWeInit) CoUninitialize();
        id_cpu = "Ошибка прокси-бланкета WMI";
        return false;
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR bstrQuery = SysAllocString(L"SELECT Name FROM Win32_Processor");
    BSTR bstrWQL = SysAllocString(L"WQL");
    if (!bstrQuery || !bstrWQL) {
        pSvc->Release();
        if (bWeInit) CoUninitialize();
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
        if (bWeInit) CoUninitialize();
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
            int size = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1,
                                           nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                std::string utf8(size - 1, 0);
                WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1,
                                     &utf8[0], size, nullptr, nullptr);
                // size включает нулевой байт; utf8 уже size-1 без него
                cpuNames.push_back(utf8);
                //std::cerr << "[get_cpu] Found CPU: '" << utf8 << "'\n";
            }
        } else {
            std::cerr << "[get_cpu] Get(Name) failed or empty, hr=0x"
                      << std::hex << getRes << " vt=" << vtProp.vt << "\n";
        }
        VariantClear(&vtProp);
        pclsObj->Release();
        pclsObj = nullptr;
    }
    pEnumerator->Release();

    if (bWeInit) CoUninitialize();

//    std::cerr << "[get_cpu] Total CPUs found: " << cpuNames.size() << "\n";

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

    // --- Очистка строки ---
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

    // Нормализация пробелов
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
        // Убираем ведущие/хвостовые пробелы
        if (!res.empty() && res.back() == ' ') res.pop_back();
        if (!res.empty() && res.front() == ' ') res.erase(0, 1);
        s = std::move(res);
    };
    normalize_spaces(id_cpu);

//    std::cerr << "[get_cpu] Final id_cpu='" << id_cpu << "'\n";

    return !id_cpu.empty();
}
