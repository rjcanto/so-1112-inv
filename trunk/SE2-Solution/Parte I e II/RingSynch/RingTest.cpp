#include <process.h>
#include <stdio.h>
#include <tchar.h>
#include "RingSynch.h"

#define NR_THREADS 18
#define CAPACITY 3
PRing ring;
int nr;

unsigned int __stdcall Put(void* pArguments)
{
	RingPut(ring, (LPVOID)nr++);
  return 0;
}

unsigned int __stdcall Get(void* pArguments)
{
	RingGet(ring);
  return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE multHandle[NR_THREADS];
	int i;
	ring = (PRing)malloc(sizeof(Ring));
	RingInit(ring, CAPACITY);

	for(i = 0; i < NR_THREADS/2; i++)
	{
    multHandle[i] = (HANDLE)_beginthreadex(NULL
            , 0
            , &Put
            , NULL
            , NULL
            , NULL
        );
	}
	for(i = NR_THREADS/2; i < NR_THREADS; i++)
    multHandle[i] = (HANDLE)_beginthreadex(NULL
            , 0
            , &Get
            , NULL
            , NULL
            ,NULL
    );

	WaitForMultipleObjects(NR_THREADS, multHandle, TRUE, INFINITE);

	free(ring);

	printf("Simulacao terminada!!");
	getchar();
	return 0;
}