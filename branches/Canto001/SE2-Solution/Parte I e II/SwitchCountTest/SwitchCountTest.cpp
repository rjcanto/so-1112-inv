// SwitchCountTest.cpp : Defines the entry point for the console application.
//
#define DEBUG

#include <crtdbg.h>
#include <stdio.h>
#include "..\Include\Uthread.h"

#define MAX_THREADS 3
ULONG Test1_Count;
ULONG SwitchC = 7;

VOID Test1_Thread (UT_ARGUMENT Argument) {
	printf("Start Test %c: \n", (char*)Argument );

	UtYield();
	
	printf("Terminate Test %c: \n", (char*)Argument);
	++Test1_Count;
}

VOID Test1 ()  {
	ULONG Index;

	
	Test1_Count = 0; 

	printf("\n :: Test 1 - BEGIN :: \n\n");

	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test1_Thread, (UT_ARGUMENT) ('0' + Index));
	}   

	UtRun();

	_ASSERTE(Test1_Count == MAX_THREADS);
	printf("\n\n :: Test 1 - END :: \n");
}

int main()
{
	UtInit();

	 
	Test1();
	getchar();
	
	 
	UtEnd();
	_ASSERTE(SwitchC == UtGetSwitchCount());
	printf("Teste com sucesso.\n");
  printf("Prima uma tecla para terminar!..");
	getchar();
	return 0;
}

