
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

//#define NO_COINITIALIZE
#define NO_COSECURITY
//#define NO_COSECURITY2

DWORD WINAPI CheckCompatibleSoundBoard(char *BoardName, DWORD BoardNameLength)
{
    HRESULT hres;

    *BoardName = 0;

#ifndef NO_COINITIALIZE
    //hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    hres = CoInitialize(0);

    if (FAILED(hres))
    {
        return hres;                  // Program has failed.
    }
#endif
    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------
#ifndef NO_COSECURITY
    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

    if (FAILED(hres))
    {
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                    // Program has failed.
    }
#endif
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if(FAILED(hres))
    {
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                // Program has failed.
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------
#ifndef NO_COSECURITY2
    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();     
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;               // Program has failed.
    }
#endif
    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, Sound Board
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_SoundDevice"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if(FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
   
    while(pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(SUCCEEDED(hr))
        {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            //is it working?
            hr = pclsObj->Get(L"ConfigManagerErrorCode", 0, &vtProp, 0, 0);
            if(vtProp.uintVal != 0)
                continue;

            //
            
            // Get the value of the ProductName property
            hr = pclsObj->Get(L"ProductName", 0, &vtProp, 0, 0);
            if(SUCCEEDED(hr))
            {
                if(*vtProp.bstrVal)
                    sprintf(BoardName, "%S", vtProp.bstrVal);
                else
                    sprintf(BoardName, "unknown sound board");
                VariantClear(&vtProp);
                uReturn = 0;
            }
            else
                uReturn = 2;
        }
        else
        {
            uReturn = hr;
            break;
        }
    }

    // Cleanup
    // ========
    
    if(pSvc)
        pSvc->Release();

    if(pLoc)
        pLoc->Release();

    if(pEnumerator)
        pEnumerator->Release();

    if(pclsObj)
        pclsObj->Release();
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif

    return uReturn;   // Program successfully completed.
}

DWORD CheckCPUClock(DWORD *Clock)
{
    HRESULT hres;

    *Clock = 0;

#ifndef NO_COINITIALIZE
    //hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    hres = CoInitialize(0);

    if (FAILED(hres))
    {
        return hres;                  // Program has failed.
    }
#endif
    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------
#ifndef NO_COSECURITY
    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

    if (FAILED(hres))
    {
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                    // Program has failed.
    }
#endif
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if(FAILED(hres))
    {
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;                // Program has failed.
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------
#ifndef NO_COSECURITY2
    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();     
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;               // Program has failed.
    }
#endif
    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, CPU Speed
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if(FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif
        return hres;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
   
    //while (pEnumerator)
    //one processor is sufficient
    if(pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(SUCCEEDED(hr))
        {
            VARIANT vtProp;
            VariantInit(&vtProp);

            // Get the value of the MaxClockSpeed property
            hr = pclsObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0);
            if(SUCCEEDED(hr))
            {
                *Clock = vtProp.ulVal;
                VariantClear(&vtProp);
                uReturn = 0;
            }
            else
                uReturn = 2;
        }
        else
        {
            uReturn = hr;
        }
    }
    else
        uReturn = 1;

    // Cleanup
    // ========
    
    if(pSvc)
        pSvc->Release();

    if(pLoc)
        pLoc->Release();

    if(pEnumerator)
        pEnumerator->Release();

    if(pclsObj)
        pclsObj->Release();
#ifndef NO_COINITIALIZE
        CoUninitialize();
#endif

    return uReturn;   // Program successfully completed.
}

DWORD CheckRAM(DWORD *MegaRAM)
{
    MEMORYSTATUSEX ms;

    ms.dwLength = sizeof(ms);

    GlobalMemoryStatusEx(&ms);

    *MegaRAM = ms.ullTotalPhys / (1024 * 1024);
    return 0;
}

