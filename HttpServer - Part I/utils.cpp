#include "stdafx.h"
#include "httpserver.h"

TCHAR *GetFileExtensionPos(TCHAR *path) {
	int i, size;

	size= _tcslen(path);
	for (i= size-1; i >= 0; --i)
		if (path[i] == '.')
			return &path[i+1];
	return &path[size-1];
}


int splitLine(char *line, char *words[], char delim, int nlines) {
	char c;
	int i=0, j=0, wsize;
	char *word;
 
	while ((c = line[i]) != 0) {
		while (c == delim) c = line[++i];
		wsize=0;
		word = line+i;
		while (c != 0 && c != delim) { c= line[++i]; wsize++; }
		if (wsize) words[j++] = word;
		if (c== 0 || j == nlines) break;
		line[i++] = 0;
	}
	return j;
}

/*
 * Pool implementation
 */
void PoolInit(PPool p) {
	p->poolPosition = 0;
}

void* PoolAlloc(PPool p, int size) {
	char * paux  = p->pool + p->poolPosition;
	if ((MAXPOOL - p->poolPosition) < size)
		return NULL; /* request can´t be satisfied */
	p->poolPosition += size;
	return paux;
}

char *PoolStrDup(PPool p, const char *str) {
	char *nstr = (char *) PoolAlloc(p, strlen(str) +1);
	if (nstr == NULL) return NULL;
	strcpy(nstr, str);
	return nstr;
}