// TrackingServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"


/*Global Status*/
static Logger log;	/* the Logger */

CRITICAL_SECTION mutex;

/*
the I/O Completion Port
*/
HANDLE completionPort;

UINT WINAPI ProcessConnection(LPVOID arg) {
    SOCKET connectSocket = (SOCKET) arg;
    Connection connection;
	
    ConnectionInit(&connection, connectSocket, &log);

    /* Associate Connection Socket to Completion Port */
    if (!CreateIoCompletionPort((HANDLE) connectSocket,completionPort, INPUT_OPER, (DWORD) MAX_THREADS)) {
        LoggerMessage(&log, "Error associating device to IO completion port!\n");
        return 5;
    }

    LoggerMessage(&log, "Start connection processing");
    ProcessRequest(&connection);
    LoggerMessage(&log, "End connection processing");
    closesocket(connectSocket);
    return 0;
}

VOID CreateThreadPool() {
	for (int i=0; i < MAX_THREADS; ++i) {
		_beginthreadex(NULL, 0, RunOperation,NULL,0, NULL);
	}
}

UINT WINAPI RunOperation(LPVOID arg) {
    DWORD transferedBytes;
    DWORD key;
    Connection *ConnectionsList;

    while (TRUE) {
        if (!GetQueuedCompletionStatus(
                            completionPort
                            , &transferedBytes
                            , &key
                            , (OVERLAPPED **) &ConnectionsList
                            , INFINITE))
            return 0;
        EnterCriticalSection(&mutex);
        switch (key) {
        case INPUT_OPER:
            ProcessOutputRequest(ConnectionsList, completionPort);
            break;
        case OUTPUT_OPER:
            ProcessInputRequest(ConnectionsList, completionPort);
            break;
        }
        LeaveCriticalSection(&mutex);
    }	
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
        LoggerMessage(&log, "Error creation IO completion port!");
        return 4;
    }

    /*	Follow the standard server socket/bind/listen/accept sequence */
    srvSock = socket(PF_INET, SOCK_STREAM, 0);
    if (srvSock == INVALID_SOCKET) {
        LoggerMessage(&log, "Failed server socket() call");
        return 3;
    }

    /* Associate Server Socket to Completion Port */
    if (!CreateIoCompletionPort((HANDLE) srvSock,completionPort, OUTPUT_OPER, (DWORD) MAX_THREADS)) {
        LoggerMessage(&log, "Error associating device to IO completion port!\n");
        return 5;
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

        ProcessConnection((LPVOID) connectSock);

    }

    shutdown (srvSock, SD_BOTH); /* Disallow sends and receives */
    closesocket (srvSock);
    WSACleanup();
    LoggerStop(&log);
    LoggerClose(&log);

    return 0;
}

