#include "stdafx.h"
#include "httpserver.h"


/*
 * wrappers for socket functions 
 */
void ConnectionFillBufferFromSocket(PHttpConnection c) {
	c->len= recv(c->socket, c->bufferComm, BUFFERSIZE, 0);
	c->rwPos=0;
}

static void ConnectionClearBuffer(PHttpConnection c) {
	c->rwPos=0;
	c->len=0;
}

void ConnectionFlushBufferToSocket(PHttpConnection c) {
	if (c->rwPos > 0) {
		send(c->socket, c->bufferComm, c->rwPos, 0);
		ConnectionClearBuffer(c);
	}
}



/*
 * reads a line (terminate with \r\n pair) from the connection socket
 * using the buffer the I/O buffer in PHttpConnection "cn"
 *
 * Return the number of line readed
 */ 
int ConnectionGetLine(PHttpConnection cn, char *buffer, int bufferSize) {
	int i=0;
	int c;

	while (i < bufferSize - 1 && (c = cgetchar(cn)) != -1 && c != '\r')
		buffer[i++] = c;
	if (c == -1) return -1;
	if (c== '\r') cgetchar(cn); /* read line feed */
	buffer[i]=0;
	return i;
}

/*
 * I/O formatters 
 */
void ConnectionPutString(PHttpConnection cn, char *str) {
	int c;

	while ((c=*str++) != 0) cputchar(cn, c);
}

void ConnectionPutWString(PHttpConnection cn, TCHAR *str) {
	int c;

	while ((c=*str++) != 0) cputchar(cn, c);
}

void ConnectionPutInt(PHttpConnection cn, int num) {
	char ascii[32];

	ConnectionPutString(cn, _itoa(num, ascii, 10));
}

void ConnectionCopyBytes(PHttpConnection cn, LPVOID mapAddress, DWORD fSize) {

	ConnectionFlushBufferToSocket(cn);
	send(cn->socket, (const char *) mapAddress, fSize, 0);
}


void ConnectionCopyFile(PHttpConnection cn, HANDLE hFile, int fSize) {
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
	

/*
 * Connection initialization
 */

VOID  ConnectionInit(PHttpConnection c, SOCKET s) {
	ZeroMemory(c, sizeof(HttpConnection));
	c->socket=s;
}



/*
 * Initialize context parsing HTTP(GET) request
 */
BOOL ContextInit(PHttpContext ctx, PHttpConnection c) {
	char buf[512];
	char *words[128];
	
	PoolInit(&c->pool);
	ZeroMemory(ctx, sizeof(HttpContext));
	ctx->connection=c;
	ConnectionClearBuffer(c);
	HeaderListInit(&ctx->reqList);
	HeaderListInit(&ctx->respList);

	/* get request line */
	if (ConnectionGetLine(c, buf, 512) <= 0) return FALSE;
	if (splitLine(buf, words, ' ', 3) != 3) return FALSE;
	 
	if (strcmp("GET", words[0]) != 0) return FALSE;
 
	/* get Url */
	ctx->url = PoolStrDup(&c->pool, words[1]);

	/* get version */
	 
	if (splitLine(words[2], words, '/', 2) != 2) return FALSE;
	if (strcmp(words[0], "HTTP") != 0) return FALSE;
	if (splitLine(words[1], words, '.', 2) != 2) return FALSE;
	
	ctx->version.major=atoi(words[0]);
	ctx->version.minor=atoi(words[1]);

	/* save HTTP headers in context */
	while (ConnectionGetLine(c, buf, 512) > 0) { 
		char *key, *value;

		if (splitLine(buf, words, ':' , 2) != 2) return FALSE;
		key = PoolStrDup(&c->pool, words[0]);
		value = PoolStrDup(&c->pool, words[1]+1);
		HeaderInsert(&ctx->reqList, &c->pool, key, value);
	}
	ctx->connection->rwPos=0;
	return TRUE;
}


