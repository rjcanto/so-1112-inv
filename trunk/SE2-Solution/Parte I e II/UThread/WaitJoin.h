#pragma once

#include <Windows.h>
#include "UThreadInternal.h"
#include "List.h"

typedef struct _UWAIT{
	LIST_ENTRY Link;
	PUTHREAD Thread;
	HANDLE LockedThread;
}UWAIT, *PUWAIT;

static LIST_ENTRY WaitQueue;

VOID InitializeWait()
{
	InitializeListHead(&WaitQueue);
}

VOID WaitInJoin(HANDLE Thread)
{
	PUWAIT WaitBlock = (PUWAIT)malloc(sizeof(UWAIT));
	WaitBlock->Thread = (PUTHREAD)Thread;
	WaitBlock->LockedThread = UtSelf();
	InsertHeadList(&WaitQueue, &WaitBlock->Link);
	UtDeactivate();
}

VOID VerifyWait()
{
	PLIST_ENTRY next;
	PUTHREAD currThread;
	if (IsListEmpty(&WaitQueue)) 
	{
		return;
	}

	else {
		currThread = (PUTHREAD)UtSelf();
		next = WaitQueue.Flink;

		while(next!= &WaitQueue){
			if (CONTAINING_RECORD(next, UWAIT, Link)->Thread == currThread)
			{
				RemoveEntryList(next);
				UtActivate(CONTAINING_RECORD(next, UWAIT, Link)->LockedThread);
				free(CONTAINING_RECORD(next, UWAIT, Link));
				break;
			}
			next = next->Flink;
		}
	}
}
