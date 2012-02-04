#include "stdafx.h"


/*
* Used on socket buffered I/O
*/
void ConnectionFillBufferFromSocket(PConnection c) {
    
    c->len= recv(c->socket, c->bufferIn, BUFFERSIZE, 0);
    if (c->len <=0) {
        int a=2;
    }
    c->rPos = 0;
/**
    c->ioStatus.Offset = 0;
    c->ioStatus.OffsetHigh = 0;

    WSARecv(c->socket
        , &c->bufferIn
        , BUFFERSIZE
        , (LPDWORD)&c->len
        , 0
        , &c->ioStatus
        , NULL
    );
    c->rPos=0;
/**/
}

void ConnectionFlushBufferToSocket(PConnection c) {
    if (c->wPos > 0) {
        if (send(c->socket, c->bufferOut, c->wPos, 0) != c->wPos) {
            int a=3;
        }
        c->wPos =0;
    }
    
/** 
    if (c->wPos > 0)
    {
        if ( WSASend( c->socket
            ,&c->bufferOut
            ,BUFFERSIZE
            ,(LPDWORD)&c->wPos
            ,0
            ,&c->ioStatus
            ,NULL) == c->wPos)
            
            c->wPos = 0; 
    }
/**/
}

/*
* reads a line (terminate with \r\n pair) from the connection socket
* using the buffer the I/O buffer in PHttpConnection "cn"
*
* Return the number of line readed
*/ 
int ConnectionGetLine(PConnection cn, char *buffer, int bufferSize) {
    int i=0;
    int c;

    while (i < bufferSize - 1 && (c = cgetchar(cn)) != -1 && c != '\r')
        buffer[i++] = c;
    if (c == -1) 
        return -1;
    if (c== '\r') 
        c= cgetchar(cn); /* read line feed */
    buffer[i]=0;
    return i;
}


/*
* Output formatters 
*/
char *ConnectionPutString(PConnection cn, char *str) {
    int c;

    while ((c=*str++) != 0) cputchar(cn, c);
    return str;
}

wchar_t *ConnectionPutStringFromWString(PConnection cn, wchar_t *str) {
    int c;

    while ((c=*str++) != 0) cputchar(cn, c);
    return str;
}

void ConnectionPutInt(PConnection cn, int num) {
    char ascii[32];

    ConnectionPutString(cn, _itoa(num, ascii, 10));
}

void ConnectionCopyBytes(PConnection cn, LPVOID mapAddress, DWORD fSize) {
    ConnectionFlushBufferToSocket(cn);
    send(cn->socket, (const char *) mapAddress, fSize, 0);
/**
    WSASend(cn->socket
        , &cn->bufferOut
        , BUFFERSIZE
        , &fSize
        , 0
        ,
/**/
}

void ConnectionCopyFile(PConnection cn, HANDLE hFile, int fSize) {
    HANDLE hMap;
    LPVOID mapAddress;

    if ((hMap=CreateFileMapping(hFile, NULL, PAGE_READONLY, 0,fSize,NULL)) == NULL)
        return;

    if ((mapAddress = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) == NULL)
        return;
    ConnectionCopyBytes(cn, mapAddress, fSize);

    UnmapViewOfFile(mapAddress);
    CloseHandle(hMap);
}

void ConnectionPut(PConnection cn, char* format, ...) {
    va_list ap;
    char *pcurr = format, c;

    va_start(ap,format);

    while ((c = *pcurr) != 0) {
        if (c != '%') {
            if (c != '\\')
                cputchar(cn,c);
            else {
                c = *++pcurr;
                if (c == 0) break;
                if (c== 't')
                    cputchar(cn, '\t');
                else if (c== 'n') 
                    cputchar(cn, '\n');
                else if (c== 'r') 
                    cputchar(cn, '\r');
                else if (c== '0') 
                    cputchar(cn, '\0');
            }
        }
        else {
            c = *++pcurr;
            if (c == 0) break;
            switch(c) {
            case 'd':
                ConnectionPutInt(cn, va_arg(ap, int));
                break;
            case 's':
                ConnectionPutString(cn, va_arg(ap, char *));
                break;
            case 'S':
                ConnectionPutStringFromWString(cn, va_arg(ap, wchar_t *));	
                break;	
            default:
                break;
            }

        }
        pcurr++;
    }
}


/*
* Connection initialization
*/

VOID  ConnectionInit(PConnection c, SOCKET s, Logger *log) {
    ZeroMemory(c, sizeof(Connection));
    /**
    c->bufferIn = (char *)malloc(BUFFERSIZE);
    c->bufferOut = (char *)malloc(BUFFERSIZE);
    */
    c->socket=s;
    c->log = log;
}


//VOID ConnectionTest() {
//	Connection cn;
//	SOCKET s=INVALID_SOCKET;
//	TCHAR wc;
//
//	ConnectionInit(&cn, s);
//	cn.len=10;
//	
//	ConnectionInit(&cn, s);
//	ConnectionPut(&cn, "Put %d on Buffer and the message %s, and the wide message %w\n", 5, "ok", _T("great!"));
//}