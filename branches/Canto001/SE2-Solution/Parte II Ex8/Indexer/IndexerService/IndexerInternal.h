#include "Indexer.h"
#include "..\HashTable\HashTable.h"
#include "Ring.h"

#define MAXQUERIES	4

typedef struct _SERVICE{
	BOOL opened;
	TCHAR* root;
  Ring ring;
}SERVICE, *HSERVICE;

typedef struct _PATH_ELEM{
    DWORD pathSize;
    TCHAR path[MAX_PATH];
}PATH_ELEM, *HPATH_ELEM;

typedef struct _RESULTENUM{
    DWORD maxCount;
    DWORD count;
    PATH_ELEM queryResult[CAPACITY];
}RESULTENUM, *HRESULTENUM;

typedef struct _ANSWER{
  DWORD totalDataSize;
  DWORD count;
  DWORD pathSize;
	TCHAR path[MAX_PATH * CAPACITY];
}ANSWER, *HANSWER;

INT CMPFunc(VOID* elem1,VOID* elem2){
	return _tcscmp((TCHAR*)elem1, (TCHAR *)elem2);
}

VOID REMFunc(void* item)
{
    free(item);
}
TCHAR* NextResult(RESULTENUM* resultEnum)
{
    if( resultEnum->maxCount == resultEnum->count)
    {
        free(resultEnum);
        return NULL;
    }
    return resultEnum->queryResult[resultEnum->count++].path;
}

VOID InitializeQueryEnum(QUERY_ENUM* queryEnum)
{
    queryEnum->answers = NULL;
    queryEnum->getNext = NULL;
}

VOID AnswerResultToQueryEnum(ANSWER* answer, RESULTENUM* resultEnum)
{
	DWORD datasize = 0, sizeOld, i = 0;
  BYTE* path = (BYTE*)&answer->path;
  datasize = answer->pathSize;

  resultEnum->maxCount = answer->count;
  resultEnum->count = 0;
	while(datasize != 0 && i < CAPACITY)
	{
    resultEnum->queryResult[i].pathSize = datasize;
    CopyMemory(&resultEnum->queryResult[i++].path, path, datasize);
    sizeOld = datasize;
		datasize = *(path + datasize);
    path = (path + sizeOld + sizeof(DWORD));
	}
}

