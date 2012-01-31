#include <tchar.h>

#define MAXIDDIGITS 10*2 /*nr maximo de digitos(2^32) * 2bytes(TCHAR)*/

TCHAR* FilePathParser(TCHAR* filePath)
{
	TCHAR* lastOcurrence = _tcsrchr(filePath, _T('\\'));
	if (lastOcurrence == NULL) return filePath;
	return lastOcurrence + 1;
}

TCHAR* ConcatRootWithFileName(TCHAR* root, TCHAR* filePath)
{
  size_t rootLen = _tcslen(root)*sizeof(TCHAR), filePathLen = _tcslen(filePath)*sizeof(TCHAR);
  TCHAR c = _T('\\');
  TCHAR final =_T('\0');

  TCHAR* concat = (TCHAR*)malloc(rootLen + filePathLen + (2*sizeof(TCHAR)));
	CopyMemory((void*)concat, (const void*)root, rootLen);
  CopyMemory((void*)(concat + (rootLen / sizeof(TCHAR))), (const void*)&c, sizeof(TCHAR));
  CopyMemory((void*)(concat + ((rootLen + sizeof(TCHAR)) / sizeof(TCHAR))) , (const void*)filePath, filePathLen);
  CopyMemory((void*)(concat + ((rootLen + filePathLen + sizeof(TCHAR))/sizeof(TCHAR))) , (const void*)&final, sizeof(TCHAR));
  return concat;
}

VOID GenRequestClientName(DWORD procId, DWORD threadId, TCHAR* name)
{
    TCHAR* process; 
    TCHAR* thread; 
    process = (TCHAR*)malloc(MAXIDDIGITS + sizeof(TCHAR));
    thread = (TCHAR*)malloc(MAXIDDIGITS + sizeof(TCHAR));

    _itot_s(procId, process, MAXIDDIGITS/sizeof(TCHAR), 10);
    _itot_s(threadId, thread, MAXIDDIGITS/sizeof(TCHAR), 10);

    CopyMemory((void*)name, (const void*)process, _tcslen(process)*sizeof(TCHAR));
    CopyMemory((void*)(name + _tcslen(process)), (const void*)thread, (_tcslen(thread)+1)*sizeof(TCHAR));
    free(process);
    free(thread);
}