#include <Windows.h>
#include <stdio.h>
#include "List.h"

typedef struct _DATA{
	LIST_ENTRY Link;
	void * elem;
}DATA, *PDATA;

typedef  struct _Ring{
	LIST_ENTRY Queue;
	HANDLE hSemaphoreFull;
	HANDLE hSemaphoreEmpty;
	HANDLE hMutex;
	int nItems;
	int maxCapacity;
}Ring, *PRing;

/* Inicia o ring “r” com a capacidade especificada em “capacity” */
void RingInit(Ring *r, int capacity);

/* Insere mais um elemento no ring. Bloqueia a thread invocante enquanto o ring se encontrar cheio. */
void RingPut(Ring *r, void *elem);

/* Retorna o elemento mais antigo do ring “r”. Bloqueia a thread invocante enquanto o ring se encontrar vazio. */
void *RingGet(Ring *r);