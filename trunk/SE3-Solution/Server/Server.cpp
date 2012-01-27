// TrackingServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

static Logger log;	/* the Logger */
HANDLE completionPort; /* the I/O Completion Port */

UINT WINAPI ProcessConnection(LPVOID arg) {
	SOCKET connectSocket = (SOCKET) arg;
	Connection connection;
	
	ConnectionInit(&connection, connectSocket, &log);	
	LoggerMessage(&log, "Start connection processing");
	ProcessRequest(&connection);
	LoggerMessage(&log, "End connection processing");
	closesocket(connectSocket);
	return 0;
}

int _tmain (int argc, LPCTSTR argv []) {
	WSADATA WSStartData;				/* Socket library data structure   */

	/* Server listening and session sockets. */	
	SOCKET srvSock = INVALID_SOCKET, connectSock = INVALID_SOCKET;
	int addrLen;
	struct sockaddr_in srvSAddr;		/* Server's Socket address structure */
	struct sockaddr_in connectSAddr;	/* Connection socket address structure   */
	unsigned short port=SERVER_PORT;

	if (argc > 2) {
		_tprintf (_T("Usage: %s [ <port number>] "), argv[0]);
		return 1;
	}
	
	/* retrieve the port number from program arguments */
	if (argc == 2)
		port =  _ttoi (argv[1]);

	/*	Initialize the WS library. Ver 2.0 */
	if (WSAStartup (MAKEWORD (2, 0), &WSStartData) != 0) {
		_tprintf (_T("Cannot support sockets"));
		return 2;
	}
	
	/* Create and start the logger */
	LoggerCreateFromStream(&log, stdout);
	LoggerStart(&log);

	/* Store initialization */
	StoreInit();

  /* Create completion port*/
  if ((completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, MAX_THREADS)) == NULL) {
		_tprintf(_T("Error %d creation IO completion port!\n"), GetLastError());
		return FALSE;
	}
  
  /*	Follow the standard server socket/bind/listen/accept sequence */
	srvSock = socket(PF_INET, SOCK_STREAM, 0);
	if (srvSock == INVALID_SOCKET) {
		LoggerMessage(&log, "Failed server socket() call");
		return 3;
	}
    
	/*	
	 * Prepare the socket address structure for binding the
	 * server socket to port number "reserved" for this service.
	 * Accept requests from any client machine.  
	 */
	 
	srvSAddr.sin_family = AF_INET;	
	srvSAddr.sin_addr.s_addr = htonl(INADDR_ANY);    
	srvSAddr.sin_port = htons(SERVER_PORT);	
	if (bind (srvSock, (struct sockaddr *)&srvSAddr, sizeof(srvSAddr)) == SOCKET_ERROR) {
		LoggerMessage(&log, "Failed server bind() call");
		return 1;
	}
	if (listen (srvSock, MAX_PENDING_CONNECTIONS) != 0) {
		LoggerMessage(&log, "Server listen() error");
		return 1;
	}

	/* Main thread becomes listening/connecting/monitoring thread */
	/* Create a new thread to process the connection */
	while (true) {
		addrLen = sizeof(connectSAddr);
		
		LoggerMessage(&log, _T("Waiting for connection requests."));
	    connectSock = accept (srvSock, (struct sockaddr *)&connectSAddr, &addrLen);
		if (connectSock == INVALID_SOCKET) {
			LoggerMessage(&log, "accept: invalid socket error");
			break;
		}
		LoggerMessage(&log, "Connected with %s, port %d.\n", inet_ntoa(connectSAddr.sin_addr), connectSAddr.sin_port);

		_beginthreadex(NULL, 0, ProcessConnection, (LPVOID) connectSock, 0, NULL);	 
		
	}
	 
	shutdown (srvSock, SD_BOTH); /* Disallow sends and receives */
	closesocket (srvSock);
	WSACleanup();
	LoggerStop(&log);
	LoggerClose(&log);
 
	return 0;
}

