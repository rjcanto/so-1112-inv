#include "stdafx.h"

BOOL Char2Wchar(TCHAR* pDest, char* pSrc, int dstLen)
{
    int srcLen;
    int outLen = 0;

    if(pDest == NULL || pSrc == NULL)
        return FALSE;

    srcLen = strlen(pSrc);
    if(srcLen == 0 || srcLen > dstLen)
        return FALSE;

    if ((outLen =MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pSrc, -1, pDest, dstLen))==0) {
        DWORD err = GetLastError();
        return FALSE;
    }

    pDest[outLen] = '\0';
    return TRUE;
}

BOOL buildEndPoint(sockaddr_in *ep, int port, char *ipaddress) {
    memset (ep, 0, sizeof(sockaddr_in));    
    ep->sin_family = AF_INET;	
    ep->sin_addr.s_addr = inet_addr(ipaddress);
    if (ep->sin_addr.s_addr == INADDR_NONE) 
        return FALSE;
    ep->sin_port = port;
    return TRUE;
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



VOID ToUpper(char *str) {
    while( *str = toupper(*str)) str++;
}