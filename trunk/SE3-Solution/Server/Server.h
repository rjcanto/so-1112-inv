#pragma once
#include <WinSock2.h>

#define BUFFERSIZE 4096		/* I/O buffer size used during server request processing */

#define SERVER_PORT 8888   /* Well known server port. */
#define MAX_PENDING_CONNECTIONS 1024
#define MAXSIZE 1024
#define MAX_THREADS 3
#define MIN_THREADS 2
#define MAX_CONNECTIONS 2
#define MAX_INACTIVE_TIME 15000
#define RECV_PARTIAL 3
#define RECV_OPER 1
#define SEND_OPER 2
#define START_OPER 0

#ifdef __cplusplus
extern "C" {
#endif
/*
  Estrutura que representa o estado de uma ligação
*/
typedef struct Connection  {
//TODO Connection in asynchronous mode
  WSAOVERLAPPED ioStatus;
	WSABUF bufferIn;	/* buffer usado na leitura de dados da ligação */
	CHAR bufferOut[BUFFERSIZE];	/* buffer usado na escrita de dados da ligação */
	int rPos;					/* índice que identifica o que já lido do buffer */
	int wPos;					/* índice que identifica o que já escrito no buffer */
	int len;					/* número de bytes presentes no buffer(usado na leitura) */
  int lastReq;
  CHAR requestType[MAXSIZE];
  WSABUF streamBuf;
	SOCKET socket;				/* o socket de onde o buffer foi lidos ou para onde vai ser escrito */
	Logger *log;				/* processador das mensagens de log */
  int key;
} Connection, *PConnection;


/* macros for buffered char I/O */

/*/
#define cgetchar(c)  \
	((c)->rPos == (c)->len) ? \
		ReadFromSocket(c), ((c)-> == 0 ? -1 : (c)->bufferIn.buf[(c)->rPos++]) : \
		(c)->bufferIn.buf[(c)->rPos++]
/**/
#define cgetchar(c)  \
	((c)->rPos == (c)->len) ? \
		-1 : (c)->bufferIn.buf[(c)->rPos++]

#define cputchar(cn, c)  do { \
		if ((cn)->wPos == BUFFERSIZE) \
			ConnectionFlushBufferToSocket(cn);	\
	    (cn)->bufferOut[cn->wPos++] = (c); \
		} while(0)

VOID ConnectionInit(PConnection c, SOCKET s, Logger *log, int key);
VOID ConnectionEnd(PConnection c);

UINT WINAPI RunOperation(LPVOID arg) ;

void ConnectionFillBufferFromSocket(PConnection c);
void ConnectionFillBufferFromSocketUsingStreamBuf(PConnection c);
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
VOID ReadFromSocket(PConnection cn);
VOID VerifyRequestPartial(PConnection connection, DWORD transferedBytes , HANDLE completionPort);
VOID ProcessInputRequest(PConnection cn, HANDLE completionPort);
VOID ProcessOutputRequest(PConnection cn, HANDLE completionPort);
#ifdef __cplusplus
} // extern "C"
#endif