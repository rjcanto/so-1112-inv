// TestProgram.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\UThread\UThread.h"

#define MAX_THREADS 3000
#define SLEEP_TIME 1000

//int TestUTGetSwitchCount();

ULONG Test1_Count;

VOID Test1_Thread (UT_ARGUMENT Argument) {	
	UtYield() ;
	return ; 
}

VOID TestSleep_Thread (UT_ARGUMENT Argument) {
	printf("Tempo efectivo de espera: %d\n", UtSleep(SLEEP_TIME));
	return ;
}

VOID TestUTGetSwitchCount() {
	
	UtCreate(Test1_Thread, (UT_ARGUMENT) (NULL));
	UtCreate(Test1_Thread, (UT_ARGUMENT) (NULL));	
	UtCreate(Test1_Thread, (UT_ARGUMENT) (NULL));
	UtRun();
	printf("Numero de comutacoes: %d\n",UtGetSwitchCount());
}

VOID TestUtGetThreadSwitchTime() {
	for(int i = 0; i < MAX_THREADS; i++)
		UtCreate(Test1_Thread, (UT_ARGUMENT) (NULL));
	DWORD timeStart = GetTickCount();
	UtRun();
	int countSwitches = UtGetSwitchCount();
	DWORD timeEnd = GetTickCount();
	double timeElapsed = timeEnd-timeStart;
	//printf("Numero de comutacoes: %d\n",UtGetSwitchCount());
	//printf("Tempo de execução: %f\n",timeElapsed);
	printf("Tempo por comutacao: %f\n",timeElapsed/countSwitches);
}

VOID TestUtAlive() {
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	UtInit();

	//TestUTGetSwitchCount();
	for(;;)
	TestUtGetThreadSwitchTime();
	//TestUtAlive();
	
	getchar();
	UtEnd();

	return 0;
}

