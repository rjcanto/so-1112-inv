#pragma once
#include <WinSock2.h>

#define BUFFERSIZE 4096		/* I/O buffer size used during server request processing */

#define SERVER_PORT 8888   /* Well known server port. */
#define MAX_PENDING_CONNECTIONS 1024

/*
  Estrutura que representa o estado de uma liga��o
*/
typedef struct Connection  {
	CHAR bufferIn[BUFFERSIZE];	/* buffer usado na leitura de dados da liga��o */
	CHAR bufferOut[BUFFERSIZE];	/* buffer usado na escrita de dados da liga��o */
	int rPos;					/* �ndice que identifica o que j� lido do buffer */
	int wPos;					/* �ndice que identifica o que j� escrito no buffer */
	int len;					/* n�mero de bytes presentes no buffer(usado na leitura) */
	SOCKET socket;				/* o socket de onde o buffer foi lidos ou para onde vai ser escrito */
	Logger *log;				/* processador das mensagens de log */
} Connection, *PConnection;


/* macros for buffered char I/O */
#define cgetchar(c)  \
	((c)->rPos == (c)->len) ? \
		ConnectionFillBufferFromSocket(c), ((c)->len == 0 ? -1 : (c)->bufferIn[(c)->rPos++]) : \
		(c)->bufferIn[(c)->rPos++]

#define cputchar(cn, c)  do { \
		if ((cn)->wPos == BUFFERSIZE) \
			ConnectionFlushBufferToSocket(cn);	\
	    (cn)->bufferOut[cn->wPos++] = (c); \
		} while(0)

VOID ConnectionInit(PConnection c, SOCKET s, Logger *log);
 
void ConnectionFillBufferFromSocket(PConnection c);
void ConnectionFlushBufferToSocket(PConnection c);

/* I/O Formatters */
int ConnectionGetLine(PConnection cn, char *buffer, int bufferSize);
char *ConnectionPutString(PConnection cn, char *str);
void ConnectionPutInt(PConnection cn, int num);
void ConnectionPut(PConnection cn, char* format, ...);
wchar_t *ConnectionPutStringFromWString(PConnection cn, wchar_t *str);
void ConnectionClearBuffer(PConnection c);

/* utilitary functions */
int splitLine(char *line, char *words[], char delim, int nlines);
BOOL Char2Wchar(TCHAR* pDest, char* pSrc, int dstLen);
BOOL buildEndPoint(sockaddr_in *ep, int port, char *ipaddress);
VOID ToUpper(char *str);


/* Handler entry point */
VOID ProcessRequest(PConnection cn);