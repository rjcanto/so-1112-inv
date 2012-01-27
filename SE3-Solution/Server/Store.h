#pragma once

#include "HashTable.h"

#define STORESIZE 1024
 

/*----------------------------------------------------------
  Public Interface
-----------------------------------------------------------*/

void StoreInit();

/*
 * Register the given file as being hosted at the given location. 
 */ 
void StoreRegister(TCHAR *fileName, sockaddr_in *client);
void StoreRegister(char *fileName, sockaddr_in *client);

/*
* Removes the given location for the given file (if both exist).
* Returns a boolean value indicating if the file's location as been unregistered successfully.
*/
BOOL StoreUnregister(TCHAR *fileName, sockaddr_in *client);
BOOL StoreUnRegister(char *fileName, sockaddr_in *client);

/*
* Gets the names of the files currently being tracked.
* returns a buffer with the tracked files' names (terminated by an empty name).
*/
TCHAR *StoreGetTrackedFiles(DWORD *keysCount);

/*  
 * Gets the locations of the given file.
 *
 * Returns an array with the tracked files' locations.
 */
sockaddr_in* StoreGetFileLocations(TCHAR *fileName, DWORD *countItems);
sockaddr_in* StoreGetFileLocations(char *fileName, DWORD *countItems);
