
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch(ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //CoInitialize(0);
        break;

    case DLL_PROCESS_DETACH:
        //CoUninitialize();
        break;

    }
    return TRUE;
}

