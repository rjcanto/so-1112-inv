#include "Ring.h"

DWORD IncrementIndex(DWORD index){
    if (index == MAXELEMENTS)
        return 0;
    return ++index;
}

/* Inicia o ring “r” com a capacidade especificada em “capacity” */
void RingInit(Ring *r)
{
	r->hSemaphoreFull = CreateSemaphore(NULL, MAXELEMENTS, MAXELEMENTS, SEMAPHOREFULL_NAME_RING);
	r->hSemaphoreEmpty = CreateSemaphore(NULL, 0, MAXELEMENTS, SEMAPHOREEMPTY_NAME_RING);
	r->hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME_RING);
  r->iPut = r->iGet = 0;
}

/* Insere mais um elemento no ring. Bloqueia a thread invocante enquanto o ring se encontrar cheio. */
VOID RingPut(Ring *r, void *elem)
{
	HANDLE hSemaphoreFull, hSemaphoreEmpty, hMutex;

	hSemaphoreFull = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_ALL_ACCESS  , FALSE, SEMAPHOREFULL_NAME_RING);
	hSemaphoreEmpty = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_ALL_ACCESS , FALSE, SEMAPHOREEMPTY_NAME_RING);
	hMutex = OpenMutex(SYNCHRONIZE|MUTEX_ALL_ACCESS , FALSE, MUTEX_NAME_RING);

	WaitForSingleObject(hSemaphoreFull, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

  CopyMemory((void*)&r->queries[r->iPut*SIZEOFELEM], elem, SIZEOFELEM);
  r->iPut = IncrementIndex(r->iPut);

	ReleaseSemaphore(hSemaphoreEmpty, 1, NULL);
  CloseHandle(hSemaphoreFull);
  CloseHandle(hSemaphoreEmpty);
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
}

/* Retorna o elemento mais antigo do ring “r”. Bloqueia o servidor enquanto o ring se encontrar vazio. */
void* RingGet(Ring *r)
{
	void* elem;
  HANDLE hSemaphoreFull, hSemaphoreEmpty, hMutex;

	hSemaphoreFull = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_ALL_ACCESS  , FALSE, SEMAPHOREFULL_NAME_RING);
	hSemaphoreEmpty = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_ALL_ACCESS , FALSE, SEMAPHOREEMPTY_NAME_RING);
	hMutex = OpenMutex(SYNCHRONIZE|MUTEX_ALL_ACCESS , FALSE, MUTEX_NAME_RING);

	WaitForSingleObject(hSemaphoreEmpty, INFINITE);
	WaitForSingleObject(hMutex, INFINITE);

  elem = malloc(SIZEOFELEM);
  CopyMemory(elem, (void*)&r->queries[r->iGet*SIZEOFELEM], SIZEOFELEM);
  r->iGet = IncrementIndex(r->iGet);

	ReleaseSemaphore(hSemaphoreFull,1, NULL);
	ReleaseMutex(hMutex);	
	return elem;
}