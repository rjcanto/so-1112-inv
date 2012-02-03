#include "stdafx.h"
#define MAXSIZE 1024

typedef int (*MessageProcessor)(PConnection cn);

/*
* Handle to process a Register Command
*
* Read message payload, terminated by an empty line. 
* Each payload line has the following format:
* <filename>:<ipAddress>:<portNumber>
*/
static int ProcessRegisterMessage(PConnection cn) {
    char line[MAXSIZE];
    int lineSize;

    while ( (lineSize =ConnectionGetLine(cn, line, MAXSIZE)) > 0)
    {
        char *triple[3];
        int nElems = splitLine(line, triple, ':', 3);
        unsigned port;
        sockaddr_in epoint;

        if (nElems != 3)
        {
            LoggerMessage(cn->log, "Handler - Invalid REGISTER message.");
            return -1;
        }

        port = atoi(triple[2]);

        if(port == 0 || port > 0xFFFF)
        {
            LoggerMessage(cn->log, "Handler - Invalid port number message.");
            return -1;
        }
        if (!buildEndPoint(&epoint, port, triple[1])) {
            LoggerMessage(cn->log, "Handler - Invalid IP address in message.");
            return -1;
        }
        StoreRegister(triple[0], &epoint);
    }
    return lineSize;
}

/*
* Handle to process an Unregister Command.
*
* Read message payload, terminated by an empty line. 
* Each payload line has the following format
* <filename>:<ipAddress>:<portNumber>
*/
static int ProcessUnregisterMessage(PConnection cn) {
    char line[MAXSIZE];
    int lineSize;

    while ( (lineSize =ConnectionGetLine(cn, line, MAXSIZE)) > 0)
    {
        char *triple[3];
        int nElems = splitLine(line, triple, ':', 3);
        unsigned port;
        sockaddr_in epoint;

        if (nElems != 3)
        {
            LoggerMessage(cn->log, "Handler - Invalid REGISTER message.");
            return -1;
        }

        port = atoi(triple[2]);

        if(port == 0 || port > 0xFFFF)
        {
            LoggerMessage(cn->log, "Handler - Invalid port number message.");
            return -1;
        }
        if (!buildEndPoint(&epoint, port, triple[1])) {
            LoggerMessage(cn->log, "Handler - Invalid IP address in message.");
            return -1;
        }
        StoreUnRegister(triple[0], &epoint);
    }
    return lineSize;
}

/*
* Handle to process ListFiles Command.
*
* return to client the list of tracked files, one per line in the form <ipaddress>:<port>
* The list is terminated by an empty line
*/
static int ProcessListFilesMessage(PConnection cn) {
    // Request message payload is empty.
    wchar_t *files, *curr;
    DWORD count;


    files = StoreGetTrackedFiles(&count);
    curr=files;
    while (*curr != 0)  {
        curr = ConnectionPutStringFromWString(cn, curr);
        cputchar(cn,'\n');
    }
    cputchar(cn,'\n');
    ConnectionFlushBufferToSocket(cn);
    free(files);
    return count;
}

/*
* Handle to process ListLocations Command.
*
* Return to client the list of locations for the files, one per line
* The list is terminated by an empty line.
*/
static int ProcessListLocationsMessage(PConnection cn) {
    // Request message payload is composed of a single line containing the file name.
    char fileName[MAX_PATH];
    int nameSize;
    sockaddr_in *endPoints;
    DWORD count, i;

    nameSize = ConnectionGetLine(cn, fileName, MAXSIZE);

    endPoints = StoreGetFileLocations(fileName, &count);
    if (endPoints==NULL) 
        return -1;
    for(i=0; i < count; ++i)
        ConnectionPut(cn, "%s:%d\n", inet_ntoa(endPoints[i].sin_addr), endPoints[i].sin_port); 
    cputchar(cn, '\n');
    ConnectionFlushBufferToSocket(cn);
    free(endPoints);
    return count;
}


static struct ProcessorRegistration {
    char *msgType; 
    MessageProcessor processor;
}  processorRegistry[] = {
    { "REGISTER", ProcessRegisterMessage },
    { "UNREGISTER", ProcessUnregisterMessage },
    { "LIST_FILES", ProcessListFilesMessage },
    { "LIST_LOCATIONS", ProcessListLocationsMessage },
    { NULL, NULL }
};

static MessageProcessor processorForMessageType(char *msgType) {
    struct ProcessorRegistration *currRegist = processorRegistry;

    while (currRegist->msgType != NULL) { 
        if (!strcmp(msgType, currRegist->msgType))
            return currRegist->processor;
        currRegist++;
    }
    return NULL;
}

VOID ProcessRequest(PConnection cn) {
    char requestType[MAXSIZE];

    int lineSize;

    while ( (lineSize = ConnectionGetLine(cn, requestType, MAXSIZE)) > 0)
    {   
        MessageProcessor processor;
        ToUpper(requestType);
        if ((processor = processorForMessageType(requestType)) == NULL)
        {
            LoggerMessage(cn->log, "Handler - Unknown message type(%s, size=%d). Servicing ending.", requestType, lineSize);
            break;
        }
        // Dispatch request processing
        LoggerMessage(cn->log, "Start process message type %s\n", requestType);
        processor(cn);
        LoggerMessage(cn->log, "End process message type %s\n", requestType);
    }
}

VOID ProcessInputRequest(PConnection cn) {
    int lineSize;

    if ( (lineSize = ConnectionGetLine(cn, cn->bufferIn.buf, cn->bufferIn.len)) > 0)
    {   
        ToUpper(cn->bufferIn.buf);
        PostQueuedCompletionStatus(completionPort, lineSize,(ULONG_PTR) INPUT_OPER,&cn->ioStatus);
    }
}

VOID ProcessOutputRequest(PConnection cn) {
    int lineSize = 0;
    MessageProcessor processor;
    
    if ((processor = processorForMessageType(cn->bufferIn.buf)) == NULL)
    {
        LoggerMessage(cn->log, "Handler - Unknown message type(%s, size=%d). Servicing ending.", cn->bufferOut.buf, lineSize);
        return;
    }
    // Dispatch request processing
    LoggerMessage(cn->log, "Start process message type %s\n", cn->bufferOut.buf);
    lineSize = processor(cn);
    LoggerMessage(cn->log, "End process message type %s\n", cn->bufferOut.buf);
    PostQueuedCompletionStatus(completionPort, lineSize,(ULONG_PTR) OUTPUT_OPER,&cn->ioStatus);
}