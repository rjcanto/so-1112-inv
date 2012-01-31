// AliveTest.cpp : Defines the entry point for the console application.
//

#include <crtdbg.h>
#include <stdio.h>
#include <tchar.h>
#include "..\Include\Uthread.h"

HANDLE sleepThread;
HANDLE readyThread;
HANDLE exitThread;

VOID sleep()
{
	UtSleep(20);
}

VOID test()
{
	_ASSERTE (UtAlive(sleepThread)== TRUE);
	_ASSERTE (UtAlive(UtSelf())== TRUE);
	_ASSERTE (UtAlive(readyThread)==TRUE);
}

VOID ready()
{
	_ASSERTE (UtAlive(exitThread) == FALSE);
}

int _tmain(int argc, _TCHAR* argv[])
{

	UtInit();

	sleepThread = UtCreate((UT_FUNCTION)sleep, NULL);
	exitThread = UtCreate((UT_FUNCTION)test, NULL);
	readyThread = UtCreate((UT_FUNCTION)ready, NULL);

	UtRun();

	UtEnd();
	printf("End of Simulation!\n");
	getchar();
	return 0;
}

