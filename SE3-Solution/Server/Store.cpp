/*
* INSTITUTO SUPERIOR DE ENGENHARIA DE LISBOA
* Licenciatura em Engenharia Informática e de Computadores
*
* Sistemas Operativos- Inverno 2011-2012
*
* Código base para a 3ª Série de Exercícios.
* Notas Importantes:
*   O Store fornecido não tem uma implementação thread/safe
*   Por outro lado, são aceites pares (ficheiro, endpoint) repetidos.
*/

#include "stdafx.h"
#include "HashTable.h"


#define STORESIZE 1024

static HASH_TABLE store;

static INT CmpEndPoints(VOID *val1, VOID *val2) {
    sockaddr_in *ep1 = (sockaddr_in *) val1;
    sockaddr_in *ep2 = (sockaddr_in *) val2;

    if (ep1->sin_addr.s_addr > ep1->sin_addr.s_addr) return 1;
    if (ep1->sin_addr.s_addr < ep1->sin_addr.s_addr) return -1;
    return  (int) ep1->sin_port - (int)  ep2->sin_port;
}

static VOID DeleteEndPoint(VOID *val) {
    free(val);
}

/*----------------------------------------------------------
Public Interface
-----------------------------------------------------------*/

void StoreInit() {
    HashTableInit(&store, CmpEndPoints, DeleteEndPoint, STORESIZE);
}

/*
* Register the given file as being hosted at the given location. 
*/ 
void StoreRegister(TCHAR *fileName, sockaddr_in *client) {
    sockaddr_in *dup = (sockaddr_in*) malloc(sizeof(sockaddr_in));
    *dup = *client;
    HashTableAdd(&store, fileName, dup, sizeof(sockaddr_in));
}

/*
* Register the given file as being hosted at the given location. 
*/ 
void StoreRegister(char *fileName, sockaddr_in *client) {
    TCHAR wName[MAX_PATH];

    if (!Char2Wchar(wName, fileName, MAX_PATH))
        return;
    sockaddr_in *dup = (sockaddr_in*) malloc(sizeof(sockaddr_in));
    *dup = *client;
    HashTableAdd(&store, wName, dup, sizeof(sockaddr_in));
}


/*
* Removes the given location for the given file (if both exist).
* Returns a boolean value indicating if the file's location as been unregistered successfully.
*/
BOOL StoreUnregister(TCHAR *fileName, sockaddr_in *client)
{
    return HashTableRemove(&store, fileName, client);
}

BOOL StoreUnRegister(char *fileName, sockaddr_in *client) {
    TCHAR wName[MAX_PATH];

    if (!Char2Wchar(wName, fileName, MAX_PATH))
        return FALSE;

    return HashTableRemove(&store, wName, client);
}

/*
* Gets the names of the files currently being tracked.
* returns a buffer with the tracked files' names (terminated by an empty name).
*/
TCHAR *StoreGetTrackedFiles(DWORD *keysCount)
{
    return HashTableKeys(&store, keysCount);
}

/*  
* Gets the locations of the given file.
*
* Returns an array with the tracked files' locations.
*/
sockaddr_in* StoreGetFileLocations(TCHAR *fileName, DWORD *countItems)
{
    return (sockaddr_in*) HashTableKeyValues(&store, fileName, countItems);
}

sockaddr_in* StoreGetFileLocations(char *fileName, DWORD *countItems)
{
    TCHAR wName[MAX_PATH];

    if (!Char2Wchar(wName, fileName, MAX_PATH))
        return NULL;
    return (sockaddr_in*) HashTableKeyValues(&store, wName, countItems);
}


static void  showTrackedFiles(TCHAR *files, DWORD count) {
    DWORD i=0;
    _tprintf(_T("There are %d tracked files\n"), count);
    for(i =0; i < count; ++i) {
        int fileSize = _tcslen(files);
        _tprintf(_T("\t%s\n"), files);
        files += fileSize+1;
    }
}

static void  showFileLocations(sockaddr_in *locations, DWORD count) {
    DWORD i=0;
    _tprintf(_T("There are %d file locations for file\n"), count);
    for(i=0; i < count; ++i) {
        printf("\t%s:%d\n", inet_ntoa(locations[i].sin_addr), locations[i].sin_port );
    }
}

//VOID StoreTest() {
//	sockaddr_in  endpoint;
//	DWORD count;
//
//	StoreInit();
//
//	/* add file locations */
//	buildEndPoint(&endpoint, 8000, "192.168.0.1");
//	StoreRegister(_T("f1"), &endpoint);
//	buildEndPoint(&endpoint, 8001, "192.168.0.2");
//	StoreRegister(_T("f1"), &endpoint);
//
//	buildEndPoint(&endpoint, 8002, "192.168.0.2");
//	StoreRegister(_T("f2"), &endpoint);
//
//	/* show file locations and keys */
//
//	
//	TCHAR *keys = StoreGetTrackedFiles(&count);
//	showTrackedFiles(keys, count);
//	sockaddr_in *locations = (sockaddr_in*) StoreGetFileLocations(_T("f1"), &count);
//	showFileLocations(locations, count);
//}

