#include <windows.h>
#include <comdef.h>
#include <stdio.h>



DWORD WINAPI CheckCompatibleSoundBoard(char *BoardName, DWORD BoardNameLength);
DWORD CheckCPUClock(DWORD *Clock);
DWORD CheckRAM(DWORD *MegaRAM);

int main(int argc, char *argv[])
{
    DWORD Clock=0;
    DWORD Result;
    char Buffer[1024];

    //CoInitializeEx(0, COINIT_MULTITHREADED); 

    Result = CheckCPUClock(&Clock);
    printf("Result=%08x Clock=%d\n", Result, Clock);

    Result = CheckCompatibleSoundBoard(Buffer, sizeof(Buffer));
    printf("Result=%08x Board=%s\n", Result, Buffer);

    fgetc(stdin);


    //CoUninitialize(); 

    return 0;
}