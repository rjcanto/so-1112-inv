/*
 * INSTITUTO SUPERIOR DE ENGENHARIA DE LISBOA
 * Licenciatura em Engenharia Informática e de Computadores
 *
 * Sistemas Operativos - Inverno 2011-2012
 *
 * Código base para a 3ª Série de Exercícios.
 */

#include "stdafx.h"

static BOOL LoggerCreateFromStreamAux(Logger *log, FILE *stream, BOOL streamOwner) {
	log->stream=stream;
	log->startTime = 0;
	log->streamOwner=streamOwner;
	return TRUE;
}

BOOL LoggerCreateFromStream(Logger *log, FILE *stream) {
	return LoggerCreateFromStreamAux(log,stream,FALSE);
}

BOOL LoggerCreateFromFile(Logger *log, TCHAR *fileName) {

	FILE *stream= _tfopen(fileName, _T("w+"));
	if (stream == NULL) return FALSE;
	return LoggerCreateFromStreamAux(log, stream,TRUE);
}
	
BOOL LoggerStart(Logger *log)
{
	if (log->startTime != 0) return FALSE;
	time(&log->startTime);
	log->numRequests=0;
	TCHAR *atime = _tctime(&log->startTime);
	atime[_tcslen(atime)-1]=0;
	_ftprintf(log->stream, _T("::- LOG STARTED @ %s -::\n\n"),atime );	
	return TRUE;
}

//BOOL LoggerMessage(Logger *log, TCHAR *msg) {	
//	time_t currTime;
//	
//	if (log->startTime == 0) 
//		return FALSE; /* not started logger */
//	currTime = time(NULL);
//
//	_ftprintf(log->stream, _T("{0}: {1}\n"), _wctime(&currTime), msg);	 
//	log->numRequests++;
//	return TRUE;
//}

#define MAX_SIZE 1024
BOOL LoggerMessage(Logger *log, TCHAR *format, ...) {
	TCHAR message[MAX_SIZE];
	va_list ap;
	time_t currTime;
	TCHAR *atime;

	if (log->startTime == 0) 
		return FALSE; /* not started logger */

	va_start(ap,format);
	_vstprintf(message, format, ap);
	currTime = time(NULL);
	atime = _tctime(&currTime);
	atime[_tcslen(atime)-1]=0;
	_ftprintf(log->stream, _T("%s: %s\n"), atime, message);	 
	log->numRequests++;
	return TRUE;
}

BOOL LoggerMessage(Logger *log, char *format, ...) {
	char message[MAX_SIZE];
	va_list ap;
	time_t currTime;
	char *atime;
	 
	if (log->startTime == 0) 
		return FALSE; /* not started logger */

	va_start(ap,format);
	vsprintf(message, format, ap);
	currTime = time(NULL);
	atime = ctime(&currTime);
	atime[strlen(atime)-1]=0;
	fprintf(log->stream, "%s: %s\n", atime, message);	 
	log->numRequests++;
	return TRUE;
}

BOOL LoggerStop(Logger *log)
{
	time_t currTime = time(NULL);
	time_t elapsed = currTime - log->startTime;
	
	if (log->startTime == 0) 
		return FALSE; /* already stopped logger */
	_ftprintf(log->stream, _T("Running for %d second(s)\n"),elapsed);	 
	_ftprintf(log->stream, _T("Number of request(s): %d\n\n"), log->numRequests);	 
	
	_ftprintf(log->stream, _T("::- LOG STOPPED @ {0} -::\n\n"), currTime);	 
	fflush(log->stream);
	log->startTime=0;
	return TRUE;
}
	 
BOOL LoggerClose(Logger *log) {
	if (log->startTime != 0) 
		return FALSE; /* Logger must be stopped first */
	if (log->streamOwner) {
		fclose(log->stream);
		log->stream=NULL;
	}
	return TRUE;
}


//VOID LoggerTest() {
//	Logger log;
//
//	LoggerCreateFromStream(&log, stdout);
//	LoggerStart(&log);
//
//	LoggerMessage(&log, _T("Message %d"), 1);
//	Sleep(5000);
//	LoggerMessage(&log, _T("Message %d"), 2);
//	LoggerStop(&log);
//	Sleep(5000);
//	LoggerStart(&log);
//	LoggerMessage(&log, _T("First message after Restart: %s?"), _T("is Ok"));
//
//	LoggerStop(&log);
//	LoggerClose(&log);
//}