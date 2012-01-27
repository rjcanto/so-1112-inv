#include "RingSynch.h"

/* Inicia o ring “r” com a capacidade especificada em “capacity” */
void RingInit(Ring *r, int capacity)
{
	r->hSemaphoreFull = CreateSemaphore(NULL, capacity, capacity, NULL);
	r->hSemaphoreEmpty = CreateSemaphore(NULL, 0, capacity, NULL);
	r->hMutex = CreateMutex(NULL, FALSE, NULL);
	InitializeListHead(&r->Queue);
	r->maxCapacity =  capacity;
	r->nItems = 0;
}

/* Insere mais um elemento no ring. Bloqueia a thread invocante enquanto o ring se encontrar cheio. */
void RingPut(Ring *r, void *elem)
{
	PDATA data;
	
	WaitForSingleObject(r->hSemaphoreFull, INFINITE);
	WaitForSingleObject(r->hMutex, INFINITE);
	data = (PDATA) malloc((sizeof(DATA)));
	data->elem = elem;
	InsertTailList(&r->Queue, &data->Link);
	r->nItems++;
	printf("Valor inserido= %p\n",data->elem);
	ReleaseSemaphore(r->hSemaphoreEmpty, 1, NULL);
	ReleaseMutex(r->hMutex);
}

/* Retorna o elemento mais antigo do ring “r”. Bloqueia a thread invocante enquanto o ring se encontrar vazio. */
void* RingGet(Ring *r)
{
	void* elem;
	PDATA data;

	WaitForSingleObject(r->hSemaphoreEmpty, INFINITE);

	WaitForSingleObject(r->hMutex, INFINITE);
	data = CONTAINING_RECORD(r->Queue.Flink, DATA, Link);
	RemoveHeadList(&r->Queue);
	elem = data->elem;
	r->nItems--;
	printf("Valor retornado= %p \n", elem);
	free(data);
	ReleaseSemaphore(r->hSemaphoreFull,1, NULL);
	ReleaseMutex(r->hMutex);	
	return elem;
}

