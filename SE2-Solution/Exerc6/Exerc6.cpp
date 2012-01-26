// Exerc6.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include "stdafx.h"

#define MAX_THREADS 3000


HANDLE* threadsArray =NULL ;

void threadFunc() {

}

int _tmain(int argc, _TCHAR* argv[])
{
    	
	for(int i =0; i < MAX_THREADS;i++);
		threadsArray = _beginthreadex();
	//SuspendThread(HANDLE thread)
	//ResumeThread(HANDLE thread)
	return 0;
}

