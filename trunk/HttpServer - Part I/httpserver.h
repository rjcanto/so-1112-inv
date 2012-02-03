#pragma once

#define MAXPOOL 4096		/* size of allocation pool used during processong of http (GET) requests */
#define BUFFERSIZE 4096		/* I/O buffer used during http( GET) request processing */

#define SERVER_PORT 50000   /* Well known server port. */
#define MAX_CLIENTS 1024

/*********************************
* ALTERAÇÕES NOSSAS
*********************************/
#define TIMEOUT_ERROR		258	/* Numero do erro de timeout GetQueuedCompletionStatus obtido com o GetLastError */
#define MIN_WORKINGTHREADS	1	/* Número de threads mínimo da pool */
#define MAX_WORKINGTHREADS	16	/* Número de threads máximo da pool */

/* buffered char I/O */
#define cgetchar(c)  \
	(c->rwPos == c->len) ? \
		ConnectionFillBufferFromSocket(c), (c->len == 0 ? -1 : c->bufferComm[c->rwPos++]) : \
		c->bufferComm[c->rwPos++]

#define cputchar(cn, c)  do { \
		if ((cn)->rwPos == BUFFERSIZE) \
			ConnectionFlushBufferToSocket(cn);	\
	    (cn)->bufferComm[cn->rwPos++] = (c); \
		} while(0)
 

/*
 * Pool for efficient allocations during http request processing
 */
typedef struct Pool {
	int poolPosition;
	char pool[MAXPOOL];
} Pool, *PPool;

void PoolInit(PPool p);
void *PoolAlloc(PPool, int size);
char *PoolStrDup(PPool p, const char *str);

/*
 * Http headers management list
 */
typedef struct HeaderNode {
	struct	HeaderNode *next;
	char	*headerKey;
	char	*headerValue;
} HeaderNode;

void HeaderListInit(HeaderNode *head);
void HeaderInsert(HeaderNode *head, PPool p, char *key, char *value);
char *HeaderGet(HeaderNode *head, char *key);
char *GetContentType(TCHAR *key);
	 

/*
 * Types and functions related with connection and context of http(GET)  request 
 */

typedef struct ProtocolVersion {
	BYTE major;
	BYTE minor;
} ProtocolVersion;

struct HttpConnection;

typedef struct HttpContext {
	ProtocolVersion version;
	char *url;
	HeaderNode reqList;
	HeaderNode respList;
	struct HttpConnection *connection;
} HttpContext, *PHttpContext;
	
typedef struct HttpConnection  {
	Pool pool;
	CHAR bufferComm[BUFFERSIZE];
	int rwPos;
	int len; 
	SOCKET socket;
	HttpContext ctx;
} HttpConnection, *PHttpConnection;

VOID ConnectionInit(PHttpConnection c, SOCKET s);
BOOL ContextInit(PHttpContext ctx, PHttpConnection c);

void ConnectionFillBufferFromSocket(PHttpConnection c);
void ConnectionFlushBufferToSocket(PHttpConnection c);

int ConnectionGetLine(PHttpConnection cn, char *buffer, int bufferSize);
void ConnectionPutString(PHttpConnection cn, char *str);
void ConnectionPutInt(PHttpConnection cn, int num);
void ConnectionPutWString(PHttpConnection cn, TCHAR *str);

void ConnectionCopyFile(PHttpConnection cn, HANDLE hFile, int fSize);


/* utilitary functions */
int splitLine(char *line, char *words[], char delim, int nlines);
TCHAR *GetFileExtensionPos(TCHAR *path);