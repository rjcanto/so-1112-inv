#pragma once

#include <stdio.h>
#include <Windows.h>

typedef struct Logger {
	FILE *stream;
	BOOL streamOwner;
	int numRequests;
	time_t startTime; 
} Logger;


BOOL LoggerCreateFromStream(Logger *log, FILE *stream);
BOOL LoggerCreateFromFile(Logger *log, TCHAR *fileName);
BOOL LoggerStart(Logger *log);
BOOL LoggerMessage(Logger *log, TCHAR *format, ...);

BOOL LoggerMessage(Logger *log, char *format, ...);

BOOL LoggerStop(Logger *log);
BOOL LoggerClose(Logger *log);


//VOID LoggerTest();