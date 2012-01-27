#pragma once

#ifndef INDEXERSERVICE_EXPORTS
#define INDEXERSERVICE_API __declspec(dllimport)
#else
#define INDEXERSERVICE_API __declspec(dllexport)
#endif

#include <Windows.h>
#include <tchar.h>

#define SERVICE_NAME _T("IndexerService")
#define MUTEX_NAME _T("IndexerService_Mutex")
#define SEMAPHORE_NAME _T("IndexerSevice_Semaphore")
#define CAPACITY 10

#ifdef __cplusplus
extern "C" {
#endif

extern struct RESULTENUM;

typedef struct _INDEXER{
	HANDLE hMapFile;
	HANDLE hMutex;
}INDEXER, *HINDEXER;

typedef enum	QueryType {COUNT, FOLDERS} QueryType;
typedef TCHAR* (*ENUM)(struct RESULTENUM*);

typedef struct _QUERY_ENUM
{
    struct RESULTENUM* answers;
    ENUM getNext;
}QUERY_ENUM, *HQUERY_ENUM;

typedef struct _QUERY {
	QueryType type;
	TCHAR name[MAX_PATH];
	HANDLE evt;
	DWORD clientProcId;
  TCHAR sharedResult[MAX_PATH];
} QUERY, *HQUERY;

/* funções invocadas pelo servidor */
INDEXERSERVICE_API HINDEXER CreateFileIndexer(TCHAR *root);
INDEXERSERVICE_API BOOL ProcessNextQuery(HINDEXER indexer);

/* funções invocadas pelos clientes */
INDEXERSERVICE_API HINDEXER OpenFileIndexer();
INDEXERSERVICE_API INT QueryCount(HINDEXER indexer, TCHAR *name);
INDEXERSERVICE_API QUERY_ENUM QueryFolders(HINDEXER indexer, TCHAR *name);
INDEXERSERVICE_API TCHAR *QueryNext(QUERY_ENUM queryEnum);
INDEXERSERVICE_API BOOL StopIndexer(); /* apenas pelo aplicação que força a paragem do serviço */

/* funções invocadas pelos clientes e servidor */
INDEXERSERVICE_API VOID CloseIndexer(HINDEXER indexer);

#ifdef __cplusplus
} // extern "C"
#endif