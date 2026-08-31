#include "CollectorNetwork.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <cmath> // Для lround

#pragma comment(lib, "wbemuuid.lib")

// Функция floatVal удалена за ненадобностью, 
// так как Index приходит сразу в правильном формате или конвертируется Variant напрямую.
// Если она понадобится позже, её можно вернуть сюда.

void CollectorNetwork::collect(NetworkInfo& target) {
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return;

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) { CoUninitialize(); return; }

    IWbemServices *pSvc = NULL;
    
    // ИСПРАВЛЕННАЯ СТРОКА ПОДКЛЮЧЕНИЯ:
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), 
         NULL, 
         NULL, 
         NULL, 
         0,          // <-- SecurityFlags (число)
         L"",        // <-- Authority (строка)
         0, 
         &pSvc);
         
    if (FAILED(hres)) { 
        pLoc->Release(); 
        CoUninitialize(); 
        return; 
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = NULL;
    
    // Query Computer Name
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Name FROM Win32_ComputerSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (!FAILED(hres)) {
        IWbemClassObject *pclsObj = NULL; ULONG uReturn = 0;
        while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            _variant_t vtProp; pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            target.hostname = static_cast<const char*>(_bstr_t(vtProp)); VariantClear(&vtProp); pclsObj->Release();
        }
        pEnumerator->Release();
    }

    // Query Network Adapters
    hres = pSvc->ExecQuery(bstr_t("WQL"), 
        bstr_t("SELECT Description, MACAddress, Index FROM Win32_NetworkAdapter WHERE PhysicalAdapter=TRUE AND NetEnabled=TRUE AND AdapterType LIKE '%Ethernet%'"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    if (!FAILED(hres)) {
        IWbemClassObject *pclsObj = NULL; ULONG uReturn = 0;
        while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            _variant_t vtDesc, vtMAC, vtIndex;
            pclsObj->Get(L"Description", 0, &vtDesc, 0, 0);
            pclsObj->Get(L"MACAddress", 0, &vtMAC, 0, 0);
            pclsObj->Get(L"Index", 0, &vtIndex, 0, 0);

            NetworkAdapter adapter;
            adapter.name = static_cast<const char*>(_bstr_t(vtDesc));
            if (vtMAC.vt != VT_NULL) adapter.mac_address = static_cast<const char*>(_bstr_t(vtMAC));
            
            int index = V_I4(&vtIndex); 

            char ip_query[256];
            snprintf(ip_query, sizeof(ip_query), "SELECT IPAddress FROM Win32_NetworkAdapterConfiguration WHERE Index = %d", index);
            
            IEnumWbemClassObject* pIPenum = NULL;
            hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(ip_query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pIPenum);
            
            if (!FAILED(hres) && pIPenum) {
                IWbemClassObject *pIPobj = NULL; ULONG uIPreturn = 0;
                while (pIPenum->Next(WBEM_INFINITE, 1, &pIPobj, &uIPreturn) == S_OK) {
                    _variant_t vtIP; pIPobj->Get(L"IPAddress", 0, &vtIP, 0, 0);
                    if ((vtIP.vt & VT_ARRAY) && (vtIP.vt & VT_BSTR)) {
                        SAFEARRAY* psa = vtIP.parray; BSTR* data = NULL;
                        SafeArrayAccessData(psa, (void**)&data);
                        LONG lbound, ubound;
                        SafeArrayGetLBound(psa, 1, &lbound); SafeArrayGetUBound(psa, 1, &ubound);
                        for(LONG i = lbound; i <= ubound; ++i){
                            adapter.ip_addresses.push_back(static_cast<const char*>(_bstr_t(data[i])));
                        }
                        SafeArrayUnaccessData(psa);
                    }
                    VariantClear(&vtIP); pIPobj->Release();
                }
                pIPenum->Release();
            }
            target.adapters.push_back(adapter);
            VariantClear(&vtDesc); VariantClear(&vtMAC); VariantClear(&vtIndex); pclsObj->Release();
        }
        pEnumerator->Release();
    }

    pSvc->Release(); pLoc->Release(); CoUninitialize();
}