// SocketServer1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
 #include "httpserver.h"

static  LPCTSTR rootDir;


static void ReportError(TCHAR *msg, int error) {
	_tprintf(_T("Error %d %s\n"), error, msg);
}

static char *html1 = "<html><head><title>404 Not Found</title></head><body><h2>Error: file \"";
static char *html2 = "\" not found on this server";
static char *html3 = "</h2></body></html>";

static VOID SendFileNotFound(PHttpContext ctx) {
	int i;
	ConnectionPutString(ctx->connection, "HTTP/1.1 ");
	ConnectionPutInt(ctx->connection, 404);
	ConnectionPutString(ctx->connection, " Not Found\r\n");
	ConnectionPutString(ctx->connection,"Content-Type: text/html\r\n");
	ConnectionPutString(ctx->connection,"Content-Length: ");
	ConnectionPutInt(ctx->connection, strlen(ctx->url) + strlen(html1)+strlen(html2)+128*6+strlen(html3));
	cputchar(ctx->connection, '\r');
	cputchar(ctx->connection, '\n');
	cputchar(ctx->connection, '\r');
	cputchar(ctx->connection, '\n');
	ConnectionPutString(ctx->connection, html1);
	ConnectionPutString(ctx->connection, ctx->url);
	ConnectionPutString(ctx->connection, html2);
	for (i=0; i < 128; ++i)
		ConnectionPutString(ctx->connection, "&nbsp;");
	ConnectionPutString(ctx->connection, html3);
	ConnectionFlushBufferToSocket(ctx->connection);
}

static void SendResponseFile(PHttpContext ctx, TCHAR *path, int size) {
	/* send response status */
	HANDLE hFile;

	HeaderNode *curr = ctx->respList.next;
	if ((hFile=CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		ReportError(_T("Opening File"), GetLastError());
		SendFileNotFound(ctx);
		return;
	}
	ConnectionPutString(ctx->connection, "HTTP/1.1 ");
	ConnectionPutInt(ctx->connection, 200);
	ConnectionPutString(ctx->connection, " OK\r\n");
	
	while (curr != &ctx->respList) {
		ConnectionPutString(ctx->connection,curr->headerKey);
		cputchar(ctx->connection, ':');
		cputchar(ctx->connection, ' ');
		ConnectionPutString(ctx->connection,curr->headerValue);
		cputchar(ctx->connection, '\r');
		cputchar(ctx->connection, '\n');
		curr = curr->next;
	}
	cputchar(ctx->connection, '\r');
	cputchar(ctx->connection, '\n');
	ConnectionCopyFile(ctx->connection, hFile, size);
	CloseHandle(hFile);
}

VOID ProcessRequest(PHttpContext ctx) {
	char asciiSize[32];
	DWORD i;
	TCHAR urlw[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
	TCHAR path[MAX_PATH];

	for (i=0; i <= strlen(ctx->url); i++)
		urlw[i] = ctx->url[i];
	 
	_stprintf(path, _T("%s%s"), rootDir, urlw);

	if (! GetFileAttributesEx(path, GetFileExInfoStandard, &fileAttributes) ||
		 (fileAttributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
	     /* doesn´t exist or is a directory */
		DWORD err = GetLastError();
		SendFileNotFound(ctx);
		ReportError(_T("Getting File Attributes"), err);
		return;
	}

    /* is a File */
	HeaderInsert(&ctx->respList, &ctx->connection->pool, "Content-Length", 
				PoolStrDup(&ctx->connection->pool, _itoa(fileAttributes.nFileSizeLow, asciiSize,10)));
	HeaderInsert(&ctx->respList, &ctx->connection->pool, "Content-Type", 
				GetContentType(GetFileExtensionPos(path)));
	SendResponseFile(ctx, path, fileAttributes.nFileSizeLow);
}

static void showContext(PHttpContext ctx) {
	HeaderNode *curr = ctx->reqList.next;
	printf("url=%s\n", ctx->url);
	printf("version= HTTP %d.%d\n", ctx->version.major , ctx->version.minor);

	printf("show headers:\n");
	while (curr != &ctx->reqList) {
		printf("%s:'%s'\n", curr->headerKey , curr->headerValue);
		curr = curr->next;
	}

}

UINT WINAPI ProcessConnection(LPVOID arg) {
	SOCKET connectSocket = (SOCKET) arg;
	HttpConnection connection;

	ConnectionInit(&connection, connectSocket);
	while (TRUE) {
		if (!ContextInit(&connection.ctx,  &connection) )
			break;
	
		showContext(&connection.ctx);
	 
		ProcessRequest(&connection.ctx);
			
		if (strcmp("Keep-Alive", HeaderGet(&connection.ctx.reqList, "Connection"))) {
			break;
		}
		printf("Processing done!\n");
	}
	
	closesocket(connection.socket);
	_tprintf(_T("release connection!\n"));
	
	return 0;
}

int _tmain (int argc, LPCTSTR argv [])
{

	/* get root directory */
	if (argc != 2) {
		_tprintf(_T("usage: httpserver <rootdir>\n"));
		return 1;
	}

	rootDir = argv[1];

	/* Server listening and connected sockets. */
	
	SOCKET SrvSock = INVALID_SOCKET, connectSock = INVALID_SOCKET;
	int addrLen;
	struct sockaddr_in srvSAddr;		/* Server's Socket address structure */
	struct sockaddr_in connectSAddr;	/* Connected socket with client details   */
	WSADATA WSStartData;				/* Socket library data structure   */
    BOOL terminate=FALSE;

	/*	Initialize the WS library. Ver 2.0 */
	if (WSAStartup (MAKEWORD (2, 0), &WSStartData) != 0) {
		_tprintf (_T("Cannot support sockets"));
		return 1;
	}
	 
	/*	Follow the standard server socket/bind/listen/accept sequence */
	SrvSock = socket(PF_INET, SOCK_STREAM, 0);
	if (SrvSock == INVALID_SOCKET) {
		_tprintf (_T("Failed server socket() call"));
		return 1;
	}
    
	/*	Prepare the socket address structure for binding the
	    	server socket to port number "reserved" for this service.
	    	Accept requests from any client machine.  */
	 
	srvSAddr.sin_family = AF_INET;	
	srvSAddr.sin_addr.s_addr = htonl(INADDR_ANY);    
	srvSAddr.sin_port = htons(SERVER_PORT);	
	if (bind (SrvSock, (struct sockaddr *)&srvSAddr, sizeof(srvSAddr)) == SOCKET_ERROR) {
		_tprintf (_T("Failed server bind() call"));
		return 1;
	}
	if (listen (SrvSock, MAX_CLIENTS) != 0) {
		_tprintf (_T("Server listen() error"));
		return 1;
	}

	/* Main thread becomes listening/connecting/monitoring thread */
	/* Find an empty slot in the server thread arg array */
	while (!terminate) {
		addrLen = sizeof(connectSAddr);
		_tprintf(_T("waiting  connection..\n"));
	    connectSock = accept (SrvSock, (struct sockaddr *)&connectSAddr, &addrLen);
		if (connectSock == INVALID_SOCKET) {
			_tprintf (_T("accept: invalid socket error"));
			terminate=TRUE;
			continue;
		}
		_tprintf(_T("connected with %X, port %d..\n"), connectSAddr.sin_addr, connectSAddr.sin_port);
		_beginthreadex(NULL, 0, ProcessConnection, (LPVOID) connectSock, 0, NULL);
	}
	 
 
	shutdown (SrvSock, SD_BOTH); /* Disallow sends and receives */
	closesocket (SrvSock);
	WSACleanup();
 
	return 0;
}

