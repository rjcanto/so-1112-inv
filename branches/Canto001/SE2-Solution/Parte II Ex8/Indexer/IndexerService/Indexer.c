#include "IndexerInternal.h"
#include "Utils.h"
#include <process.h>

static 	HASH_TABLE hashTable;
static HANDLE hLastMapRequest;

typedef struct _ARGLIST{
	TCHAR* root;
	HINDEXER hIndexer;
}ARGLIST, *HARGLIST;

/*Funções auxiliares ao tratamento das alterações na directoria */
typedef VOID  (*FNI)(FILE_NOTIFY_INFORMATION*, TCHAR*, HINDEXER);

VOID FileActionAdded(FILE_NOTIFY_INFORMATION* ni, TCHAR* root, HINDEXER hIndexer)
{
    TCHAR* fileName;
    TCHAR* path = NULL;
    fileName = (TCHAR*)malloc((ni->FileNameLength + 1)*2);
    CopyMemory((void*)fileName, (const void*)ni->FileName, ni->FileNameLength);
    fileName[ni->FileNameLength/2] = '\0';
    path = ConcatRootWithFileName(root, fileName);
    WaitForSingleObject(hIndexer->hMutex, INFINITE);
    HashTableAdd(&hashTable, FilePathParser(fileName), path , (_tcslen(path)+1)*2);
    free(fileName);
    ReleaseMutex(hIndexer->hMutex);
}
VOID FileActionRemoved(FILE_NOTIFY_INFORMATION* ni, TCHAR* root, HINDEXER hIndexer)
{
    TCHAR* fileName;
    TCHAR* path=NULL;
    fileName = (TCHAR*)malloc((ni->FileNameLength + 1)*2);
    CopyMemory((void*)fileName, (const void*)ni->FileName, ni->FileNameLength);
    fileName[ni->FileNameLength/2] = '\0';

    path = ConcatRootWithFileName(root, fileName);
    
    WaitForSingleObject(hIndexer->hMutex, INFINITE);
    HashTableRemove(&hashTable, FilePathParser(fileName), (void*)path);
    free(fileName);
    ReleaseMutex(hIndexer->hMutex);
}
//não aplicavel neste trabalho
VOID FileActionModified(FILE_NOTIFY_INFORMATION* ni, TCHAR* root, HINDEXER indexer)
{
	return;
}

static FNI OperName[]={
	&FileActionAdded,			// 0x00000001   
	&FileActionRemoved,			// 0x00000002   
	&FileActionModified,		// 0x00000003   
	&FileActionRemoved,			// 0x00000004
	&FileActionAdded			// 0x00000005   
};


/*Funções do tratamento das alterações na directoria */
VOID ProcessNotifyInfos(FILE_NOTIFY_INFORMATION* ni, TCHAR* root, HINDEXER hIndexer)
{
	do
	{
		OperName[ni->Action-1](ni, root, hIndexer);
		if (ni->NextEntryOffset == 0) 
			break;
		ni = (FILE_NOTIFY_INFORMATION *) (((BYTE*) ni) + ni->NextEntryOffset);
	}while(TRUE);
}

VOID FileNotifyInfo (HARGLIST hArgList)
{
	HANDLE dirHandle;
	FILE_NOTIFY_INFORMATION fileBuffer[MAX_PATH*10/sizeof(FILE_NOTIFY_INFORMATION)];
	BOOL readResult;
	DWORD nBytes;

	dirHandle=CreateFile(hArgList->root, FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
			OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,NULL);
	
	if (dirHandle==INVALID_HANDLE_VALUE)
		_tprintf(_T("Error %d opening dir\n"), GetLastError());

	
	while(TRUE) {
		readResult=ReadDirectoryChangesW(
			dirHandle,
			fileBuffer,
			sizeof(fileBuffer),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
			&nBytes,
			NULL,
			NULL);
 
		if (!readResult) 
			_tprintf(_T("Error %d reading changes!\n"), GetLastError());
		else
			ProcessNotifyInfos(fileBuffer, hArgList->root, hArgList->hIndexer);
	}
  free(hArgList);
}


/*Implementação do Serviço*/
HINDEXER CreateFileIndexer(TCHAR *root){
	HINDEXER hIndexer = NULL;
	HSERVICE service;
	HANDLE hMutex, hMapFile;
	HARGLIST hArgList;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(SERVICE), SERVICE_NAME);
	if (hMapFile == NULL)
	{
		_tprintf(_T("Could not create file mapping object (%d).\n"), GetLastError());
		return NULL;
	}
	service = (HSERVICE)MapViewOfFile(hMapFile, 
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(SERVICE)
	);
	if (service == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"), GetLastError());
		CloseHandle(hMapFile);
    return NULL;
	}

	hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (hMutex == NULL)
	{
		_tprintf(_T("CreateMutex error: %d\n"), GetLastError());
    CloseHandle(hMapFile);
    UnmapViewOfFile(service);
    return NULL;
	}

  ZeroMemory(service, sizeof(SERVICE));
	service->opened = TRUE;
	HashTableInit(&hashTable, CMPFunc, REMFunc, CAPACITY);
	service->root = root;

	hIndexer = (HINDEXER)malloc(sizeof(INDEXER));
	hIndexer->hMutex = hMutex;
	hIndexer->hMapFile = hMapFile;

  RingInit(&service->ring);
	hArgList =(HARGLIST) malloc(sizeof(ARGLIST));
	hArgList->root = root;
	hArgList->hIndexer = hIndexer;
	_beginthreadex(NULL, 0, (void *)&FileNotifyInfo, (void *)hArgList ,0,NULL); 
	return hIndexer;
}

HINDEXER OpenFileIndexer(){
	HINDEXER hIndexer = NULL;
	HSERVICE service;
	HANDLE hMutex, hMapFile;

	hMapFile =  OpenFileMapping(FILE_MAP_ALL_ACCESS,
		FALSE,
		SERVICE_NAME
	);
	if (hMapFile == NULL)
	{
		_tprintf(_T("Could not create file mapping object (%d).\n"), GetLastError());
		return NULL;
	}
	service = (HSERVICE)MapViewOfFile(hMapFile, 
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(SERVICE)
	);
	if (service == NULL)
	{
		_tprintf(_T("Could not map view of file (%d).\n"), GetLastError());
		CloseHandle(hMapFile);
    return NULL;
	}
	
	hMutex = OpenMutex(SYNCHRONIZE | MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
	if (hMutex == NULL)
	{
		_tprintf(_T("OpenMutex error: %d\n"), GetLastError());
		CloseHandle(hMapFile);
    UnmapViewOfFile(service);
    return NULL;
	}

	WaitForSingleObject(hMutex, INFINITE);
	if (!service->opened)
	{
		_tprintf(_T("Serviço não está aberto. \n"));
		CloseHandle(hMapFile);
    UnmapViewOfFile(service);
    ReleaseMutex (hMutex);
    return NULL;
	}
	ReleaseMutex (hMutex);

	hIndexer = (HINDEXER)malloc(sizeof(INDEXER));
	hIndexer->hMutex = hMutex;
	hIndexer->hMapFile = hMapFile;

	return hIndexer;
}

VOID CloseIndexer(HINDEXER indexer)
{
    CloseHandle(indexer->hMapFile);
    CloseHandle(indexer->hMutex);
    free(indexer);
}

BOOL StopIndexer()
{
	HANDLE		hMapFile, hMutex;
	HSERVICE	hService;

	hMapFile = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,
                   FALSE,
                   SERVICE_NAME);

	if (hMapFile == NULL) { 
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return FALSE;
	}

	hService = (HSERVICE) MapViewOfFile(hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,                   
		0,                   
		sizeof(SERVICE));

	if (hService == NULL) { 
		_tprintf(TEXT("Could not map view of file (%d).\n"), GetLastError()); 
		CloseHandle(hMapFile);
		return FALSE;
	}
	
	hMutex = OpenMutex(SYNCHRONIZE|MUTEX_ALL_ACCESS , FALSE, MUTEX_NAME);
	if(hMutex == NULL) {
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		UnmapViewOfFile(hService);
		CloseHandle(hMapFile);
		return FALSE;
	}
	WaitForSingleObject(hMutex, INFINITE);
		hService->opened = FALSE;
	ReleaseMutex(hMutex);

	UnmapViewOfFile(hService);
	return TRUE;
}

BOOL ProcessNextQuery(HINDEXER indexer)
{
    HSERVICE pService;
    HQUERY queryEntry;
    QueryItemResult* queryItemResult;
    DWORD totalResultSize, count;
    HANDLE clientProcess, duplicateEvent, hMapRequest;
    ANSWER* result;

    if(indexer == NULL) return FALSE;

    pService = (HSERVICE) MapViewOfFile(indexer->hMapFile,
		    FILE_MAP_ALL_ACCESS,
		    0,                   
		    0,                   
		    sizeof(HSERVICE)); 
    if (pService == NULL)
    return FALSE;

    if (!pService->opened) return FALSE;

    queryEntry = (HQUERY)RingGet(&pService->ring);

    WaitForSingleObject(indexer->hMutex, INFINITE);
		queryItemResult = HashTableGet(&hashTable, queryEntry->name, &count, &totalResultSize);
    ReleaseMutex(indexer->hMutex);

    clientProcess = OpenProcess( PROCESS_ALL_ACCESS
				, FALSE
				, queryEntry->clientProcId
				);

    DuplicateHandle( clientProcess
        , queryEntry->evt
        , GetCurrentProcess()
        , &duplicateEvent
        , PROCESS_ALL_ACCESS
        , FALSE
        , 0
		);

    //memoria partilhada
    hMapRequest = CreateFileMapping(INVALID_HANDLE_VALUE
        , NULL
        , PAGE_READWRITE
        , 0
        , sizeof(ANSWER)
        ,queryEntry->sharedResult
    );
    if(hMapRequest == NULL)
    {
        SetEvent(duplicateEvent);
        CloseHandle(clientProcess);
	      CloseHandle(duplicateEvent);
	      UnmapViewOfFile(pService);
        free(queryEntry);
        return FALSE;
    }

    result = (ANSWER*)MapViewOfFile(hMapRequest
        , FILE_MAP_ALL_ACCESS
        , 0
        , 0
        , sizeof(ANSWER)
    );
    if(result == NULL)
    {
        SetEvent(duplicateEvent);
        CloseHandle(clientProcess);
	      CloseHandle(duplicateEvent);
        CloseHandle(hMapRequest);
	      UnmapViewOfFile(pService);
        free(queryEntry);
        return FALSE;
    }     

    if(queryItemResult == NULL){
        result->totalDataSize = 0;
        result->count = 0;
    }
    else
    {
        result->totalDataSize = totalResultSize;
        result->pathSize = queryItemResult->itemSize;
        result->count = count;
        CopyMemory((void*) &result->path,&queryItemResult->itemData,totalResultSize);
        free(queryItemResult);
    }

    SetEvent(duplicateEvent);

    if (hLastMapRequest == NULL)
    {
        hLastMapRequest = hMapRequest;
    }
    else
    {
        CloseHandle(hLastMapRequest);
        hLastMapRequest = hMapRequest;
    }
    
	  CloseHandle(clientProcess);
	  CloseHandle(duplicateEvent);
    UnmapViewOfFile(result);
	  UnmapViewOfFile(pService);
    free(queryEntry);
	  return TRUE;
}

INT QueryCount(HINDEXER indexer, TCHAR *name)
{
	HSERVICE hService;
	QUERY query;
	INT count;
  HANDLE hMapRequest;
  ANSWER* result;

	hService = (HSERVICE) MapViewOfFile(indexer->hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,                   
		0,                   
		sizeof(HSERVICE)); 
	if (hService == NULL)
		return -1;

	if (!hService->opened) return -1;
    query.type = COUNT;
    query.clientProcId = GetCurrentProcessId();
    query.evt = CreateEvent(NULL, FALSE, FALSE, NULL);
    CopyMemory(query.name, name, (_tcslen(name)+1)*2);
    GenRequestClientName(GetCurrentProcessId(), GetCurrentThreadId(), (TCHAR*)&query.sharedResult);

    RingPut(&hService->ring, (void *)&query);

    WaitForSingleObject(query.evt, INFINITE);

    //memoria partilhada

    hMapRequest = OpenFileMapping(FILE_MAP_ALL_ACCESS
        , FALSE
        , query.sharedResult
        );
    if(hMapRequest == NULL)
    {
        CloseHandle(query.evt);
	      UnmapViewOfFile(hService);
        return -1;
    }

    result = (ANSWER*)MapViewOfFile(hMapRequest
        , FILE_MAP_ALL_ACCESS
        , 0
        , 0
        , 0
        );

    if (result  == NULL)
    {
        CloseHandle(query.evt);
        CloseHandle(hMapRequest);
	      UnmapViewOfFile(hService);
        return -1;
    }

    if (result->totalDataSize == 0)
        count = 0;
    else count = result->count;

    CloseHandle(query.evt);
    CloseHandle(hMapRequest);
    UnmapViewOfFile(result);
    UnmapViewOfFile(hService);

    return count;
}

QUERY_ENUM QueryFolders(HINDEXER indexer, TCHAR *name)
{
	  HSERVICE hService;
	  QUERY query;
	  RESULTENUM* result;
    ANSWER* answer;
    HANDLE hMapRequest;
    QUERY_ENUM info;

    InitializeQueryEnum(&info);
    result = (RESULTENUM*)malloc(sizeof(RESULTENUM));
	  hService = (HSERVICE) MapViewOfFile(indexer->hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,                   
		0,                   
		sizeof(HSERVICE)); 
	  if (hService == NULL)
		    return info;

	  if (!hService->opened) 
        return info;

    query.type = FOLDERS;
    query.clientProcId = GetCurrentProcessId();
    query.evt = CreateEvent(NULL, FALSE, FALSE, NULL);
    CopyMemory(query.name, name, (_tcslen(name)+1)*2);
    GenRequestClientName(GetCurrentProcessId(), GetCurrentThreadId(), (TCHAR*)&query.sharedResult);

    RingPut(&hService->ring, (void *)&query);

	  WaitForSingleObject(query.evt, INFINITE);

    hMapRequest = OpenFileMapping(FILE_MAP_ALL_ACCESS
      , FALSE
      , query.sharedResult
      );

    if(hMapRequest == NULL)
    {
        CloseHandle(query.evt);
	      UnmapViewOfFile(hService);
        return info;
    }

    answer = (ANSWER*)MapViewOfFile(hMapRequest
        , FILE_MAP_ALL_ACCESS
        , 0
        , 0
        , 0
        );

    if (answer == NULL)
    {
        CloseHandle(query.evt);
        CloseHandle(hMapRequest);
	      UnmapViewOfFile(hService);
        return info;
    }

    AnswerResultToQueryEnum(answer, result);
    CloseHandle(query.evt);
    CloseHandle(hMapRequest);
    UnmapViewOfFile(answer);
	  UnmapViewOfFile(hService);
    info.answers = result;
    info.getNext = &NextResult;
    return info;
}

TCHAR *QueryNext(QUERY_ENUM info)
{
    ENUM f = info.getNext;
    return f(info.answers);
}