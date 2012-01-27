// JoinTest.cpp : Defines the entry point for the console application.
//

#include <crtdbg.h>
#include <tchar.h>
#include <stdio.h>

#include "..\Include\Uthread.h"
HANDLE ThreadToTest;

VOID testingThread()
{
	_tprintf(_T("Start Thread: %d \n"), UtSelf());
	UtSleep(2000);
	_tprintf(_T("End Thread: %d\n"), UtSelf());
}

VOID test()
{
	printf("Simulation 1!\n");
	getchar();
	_ASSERTE(UtJoin(ThreadToTest) == 0);
	printf("Simulation 2!\n");
	getchar();
	_ASSERTE(UtJoin(ThreadToTest) == -1);
	printf("Simulation 3!\n");
	getchar();
}

int _tmain(int argc, _TCHAR* argv[])
{
	UtInit();

	ThreadToTest = UtCreate((UT_FUNCTION)testingThread, NULL);
	UtCreate((UT_FUNCTION)test, NULL);

	UtRun();

	UtEnd();
	printf("End of Simulation!\n");
	getchar();
	return 0;
}

