/////////////////////////////////////////////////////////////////
// Instituto Superior Engenharia de Lisboa
// DEETC - LEIC
//
// Sistemas Operativos
// Semestre Inverno 2011-2012
//
// Série de Exercícios 2
//   Exercício 3
//
// Authors:
//   Pedro Carreira - nº26154
//   Ricardo Canto  - nº30896
// 

#include <Windows.h>
#include "..\UThread\UThread.h"
#include "..\UThread\List.h"
 
//
// Sleep node for sleeping threads queue
//
typedef struct _SLEEP_NODE {
	struct _SLEEP_NODE *NextSleepNode;
	HANDLE     Thread;
	DWORD	   expectedTimeToEnd;
} SLEEP_NODE, *PSLEEP_NODE;

//
// Initializes the specified sleep node.
//
FORCEINLINE
VOID InitializeSleepNode(PSLEEP_NODE SleepNode, DWORD time) {

	SleepNode->NextSleepNode = NULL ;
	SleepNode->Thread = UtSelf();
	SleepNode->expectedTimeToEnd = time ;
}

