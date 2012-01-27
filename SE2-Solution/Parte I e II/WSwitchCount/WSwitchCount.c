#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>
#include <time.h>

#define NR_OF_THREADS 10000

void __stdcall Switch(void* pArguments)
{
    SwitchToThread;
}

int _tmain(int argc, TCHAR *argv[]) 
{
    int i = 0;
    HANDLE threads[NR_OF_THREADS];
    DWORD start, end;
    double time;
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    for(i = 0; i < NR_OF_THREADS; i++)
    {
        threads[i] = (HANDLE)_beginthreadex(NULL
            , 0
            , &Switch
            , NULL
            , CREATE_SUSPENDED
            , NULL
        );
        SetThreadAffinityMask(threads[i], 1);
        SetThreadPriority(threads[i], THREAD_PRIORITY_HIGHEST);
    }

    start = GetTickCount();

    for(i = 0; i < NR_OF_THREADS; i++)
    {
        ResumeThread(threads[i]);
    }


    WaitForMultipleObjects(NR_OF_THREADS, threads, TRUE, INFINITE);
    end = GetTickCount();
    time = end - start;
    printf("Tempo por comutacao: %f us\n",(time/(NR_OF_THREADS * 2))*1000);

    for(i = 0; i < NR_OF_THREADS; i++)
    {
        CloseHandle(threads[i]);
    }

    printf("Prima uma tecla para terminar!..");
    getchar();
    return 0;
}