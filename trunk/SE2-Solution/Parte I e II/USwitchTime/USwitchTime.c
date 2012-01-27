#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "..\Include\Uthread.h"

#define MAXTHREADS 1000

VOID Switch()
{
    UtYield();
}

int _tmain(int argc, _TCHAR* argv[])
{
    int i, countSwitches;
    DWORD timeStart,timeEnd;
    double timeElapsed;

    //
    //
    //Para permitir uma observação de tempos mais proxima da realidade
    //Com a minima interferencia de threads de outros processos 
    SetProcessAffinityMask(GetCurrentProcess(), 1);
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    UtInit();

    for(i = 0; i < MAXTHREADS; i++)
    {
        UtCreate((UT_FUNCTION)Switch, NULL);
    }
    
    timeStart = GetTickCount();
    UtRun();
    timeEnd = GetTickCount();
    countSwitches = UtGetSwitchCount();
    timeElapsed = timeEnd-timeStart;
    printf("Tempo por comutacao: %f us\n",(timeElapsed/countSwitches)*1000);
    printf("Prima uma tecla para terminar!..");
    getchar();
	  UtEnd();
    return 0;
}