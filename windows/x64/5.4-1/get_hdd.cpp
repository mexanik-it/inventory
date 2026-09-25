#include "main.h"
#include <windows.h>
#include <winioctl.h>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <wbemidl.h>


static unsigned long long wstrToUll(const std::wstring& s) {
    if (s.empty()) return 0;
    return _wcstoui64(s.c_str(), nullptr, 10);
}


std::wstring formatDiskSize(unsigned long long bytes) {
    const wchar_t* units[] = {L"b", L"Kb", L"Mb", L"Gb", L"Tb"};
    double size = static_cast<double>(bytes);
    int unitIndex = 0;
    while (size >= 1000.0 && unitIndex < 4) {
        size /= 1000.0;
        unitIndex++;
    }
    unsigned long long sizeInt = static_cast<unsigned long long>(size);

    wchar_t buf[32];
    int len = swprintf(buf, 32, L"%llu%ls", sizeInt, units[unitIndex]);
    return std::wstring(buf, len > 0 ? len : 0);
}


// Определяет тип диска: паттерны модели → MediaType → BusType
std::wstring detectDiskType(int mediaType, int busType, const std::wstring& model) {
    std::wstring m = model;
    std::transform(m.begin(), m.end(), m.begin(), ::towlower);

    // 1. Явные признаки SSD в названии
    if (m.find(L"ssd") != std::wstring::npos ||
        m.find(L"nvme") != std::wstring::npos ||
        m.find(L"m.2") != std::wstring::npos) {
        return L"SSD";
    }

    // 2. Явные признаки HDD в названии
    if (m.find(L"hdd") != std::wstring::npos ||
        m.find(L"hard") != std::wstring::npos) {
        return L"HDD";
    }

    // 3. WD HDD: паттерн "WD" + цифра (WD1600, WD5000, WD10EALS…)
    //    WD SSD: "WDS" + цифра (WDS500G2…) — не matches, т.к. после WD идёт S
    {
        size_t pos = m.find(L"wd");
        while (pos != std::wstring::npos) {
            size_t next = pos + 2;
            if (next < m.size() && std::iswdigit(m[next])) {
                return L"HDD";
            }
            pos = m.find(L"wd", pos + 1);
        }
    }

    // 4. Seagate HDD: паттерн "ST" + цифра (ST1000DM003, ST500LM012…)
    {
        size_t pos = m.find(L"st");
        while (pos != std::wstring::npos) {
            size_t next = pos + 2;
            if (next < m.size() && std::iswdigit(m[next])) {
                return L"HDD";
            }
            pos = m.find(L"st", pos + 1);
        }
    }

    // 5. Доверяем MediaType
    if (mediaType == 3) return L"HDD";
    if (mediaType == 4) return L"SSD";

    // 6. Эвристика по BusType
    if (busType == 11) return L"SSD";  // NVMe — почти всегда SSD

    // 7. Другие производители HDD
    if (m.find(L"hgst") != std::wstring::npos ||
        m.find(L"hitachi") != std::wstring::npos ||
        m.find(L"toshiba") != std::wstring::npos ||
        m.find(L"seagate") != std::wstring::npos) {
        return L"HDD";
    }

    return L"Unknown";
}


std::vector<DiskInfo> getDisks() {
    std::vector<DiskInfo> disks;

    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes)) return disks;

    IWbemLocator* pLoc = nullptr;
    hRes = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hRes) || !pLoc) {
        CoUninitialize();
        return disks;
    }

    IWbemServices* pSvc = nullptr;
    BSTR strResource = SysAllocString(L"ROOT\\Microsoft\\Windows\\Storage");
    hRes = pLoc->ConnectServer(strResource, nullptr, nullptr,
                               nullptr, 0, nullptr, nullptr, &pSvc);
    SysFreeString(strResource);
    if (FAILED(hRes) || !pSvc) {
        pLoc->Release();
        CoUninitialize();
        return disks;
    }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                      nullptr, RPC_C_AUTHN_LEVEL_CALL,
                      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = nullptr;
    BSTR strQuery = SysAllocString(
        L"SELECT Model, SerialNumber, Size, MediaType, BusType FROM MSFT_PhysicalDisk");
    BSTR strLang = SysAllocString(L"WQL");

    hRes = pSvc->ExecQuery(strLang, strQuery,
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                           nullptr, &pEnumerator);
    SysFreeString(strQuery);
    SysFreeString(strLang);

    if (SUCCEEDED(hRes) && pEnumerator) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn == 1) {
            DiskInfo info{};
            info.sizeBytes = 0;
            info.type = L"Unknown";

            VARIANT varModel, varSerial, varSize, varMedia, varBus;
            VariantInit(&varModel);
            VariantInit(&varSerial);
            VariantInit(&varSize);
            VariantInit(&varMedia);
            VariantInit(&varBus);

            pclsObj->Get(L"Model", 0, &varModel, nullptr, nullptr);
            pclsObj->Get(L"SerialNumber", 0, &varSerial, nullptr, nullptr);
            pclsObj->Get(L"Size", 0, &varSize, nullptr, nullptr);
            pclsObj->Get(L"MediaType", 0, &varMedia, nullptr, nullptr);
            pclsObj->Get(L"BusType", 0, &varBus, nullptr, nullptr);

            int busTypeVal = 0;
            if (varBus.vt == VT_I4) {
                busTypeVal = varBus.intVal;
            } else if (varBus.vt == VT_UI4) {
                busTypeVal = static_cast<int>(varBus.uintVal);
            } else {
                busTypeVal = -1;
            }

            if (busTypeVal == 7) {
                VariantClear(&varModel);
                VariantClear(&varSerial);
                VariantClear(&varSize);
                VariantClear(&varMedia);
                VariantClear(&varBus);
                pclsObj->Release();
                continue;
            }

            if (varModel.vt == VT_BSTR && varModel.bstrVal)
                info.model = varModel.bstrVal;
            if (varSerial.vt == VT_BSTR && varSerial.bstrVal)
                info.serialNumber = varSerial.bstrVal;

            if (varSize.vt == VT_BSTR && varSize.bstrVal)
                info.sizeBytes = wstrToUll(varSize.bstrVal);
            else if (varSize.vt == VT_UI8)
                info.sizeBytes = varSize.ullVal;

            int mediaTypeVal = 0;
            if (varMedia.vt == VT_I4) {
                mediaTypeVal = varMedia.intVal;
            } else if (varMedia.vt == VT_UI4) {
                mediaTypeVal = static_cast<int>(varMedia.uintVal);
            } else {
                mediaTypeVal = -1;
            }

            info.type = detectDiskType(mediaTypeVal, busTypeVal, info.model);

            disks.push_back(info);

            VariantClear(&varModel);
            VariantClear(&varSerial);
            VariantClear(&varSize);
            VariantClear(&varMedia);
            VariantClear(&varBus);
            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return disks;
}


std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
    str.erase(std::remove(str.begin(), str.end(), '\0'), str.end());
    return str;
}


bool TInventory::get_hdd() {
    auto disks = getDisks();
    id_hdd.clear();
    id_hdd_size.clear();
    sn_hdd.clear();

    if (disks.empty()) {
        id_hdd = "Unknown";
        id_hdd_size = "Unknown";
        sn_hdd = "unknown";
        return true;
    }

    for (size_t i = 0; i < disks.size(); ++i) {
        const auto& d = disks[i];

        id_hdd += wstring_to_string(d.model);
        id_hdd_size += wstring_to_string(d.type);
        id_hdd_size += "-";
        id_hdd_size += wstring_to_string(formatDiskSize(d.sizeBytes));

        std::string sn = wstring_to_string(d.serialNumber);
        size_t first = sn.find_first_not_of(" \t\n\r");
        size_t last  = sn.find_last_not_of(" \t\n\r");
        if (first != std::string::npos && last != std::string::npos) {
            sn = sn.substr(first, last - first + 1);
        }
        if (sn.empty()) sn = "unknown";

        sn_hdd += sn;

        if (i + 1 < disks.size()) {
            id_hdd      += " / ";
            id_hdd_size += " / ";
            sn_hdd      += " / ";
        }
    }

    return true;
}
