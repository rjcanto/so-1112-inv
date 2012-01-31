#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "List.h"
#include "..\IndexerService\Indexer.h"

#define SEMAPHOREFULL_NAME_RING _T("Ring_SemaphoreFull")
#define SEMAPHOREEMPTY_NAME_RING _T("Ring_SemaphoreEmpty")
#define MUTEX_NAME_RING _T("Ring_Mutex")
#define MAXELEMENTS 2
#define SIZEOFELEM sizeof(QUERY)

typedef  struct _Ring{
  DWORD iPut;
  DWORD iGet;
	HANDLE hSemaphoreFull;
	HANDLE hSemaphoreEmpty;
	HANDLE hMutex;
  BYTE queries[MAXELEMENTS * SIZEOFELEM];
}Ring, *PRing;

/* Inicia o ring “r” com a capacidade especificada em “capacity” */
VOID RingInit(Ring *r);

/* Insere mais um elemento no ring. Bloqueia a thread invocante enquanto o ring se encontrar cheio. */
VOID RingPut(Ring *r, void *elem);

/* Retorna o elemento mais antigo do ring “r”. Bloqueia a thread invocante enquanto o ring se encontrar vazio. */
VOID* RingGet(Ring *r);