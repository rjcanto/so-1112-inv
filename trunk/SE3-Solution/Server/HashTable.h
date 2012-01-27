/*----------------------------------------------------------------------------
   HashTable variant where can coexist multiple entries with the same key.
   Each bucket in the HashTable is a List of entries with the same key hash value.
   Easch entry in the bucket list is a list of entries with the same exact key.

   The Keys are assumed to be strings.
   Values can be of any type, so a value comparation function is given by the HashTable client.

   ISEL, 2011
-------------------------------------------------------------------------------*/

#pragma once

#include "List.h"
#include <tchar.h>

/* Value comparator func definition */
typedef INT (*VCMP)(VOID *, VOID*);

/* callback used in item removal  */
typedef VOID (*REMFUNC)(VOID *);

/* type used on hashtable queries */
typedef struct QueryItemResult {
	int itemSize;
	BYTE itemData[0];
} QueryItemResult;

/* Entry in a list of values with the same key */
typedef struct ITEMLIST_ENTRY {
	VOID *val;
	int dataSize;
	LIST_ENTRY link;
} ITEMLIST_ENTRY;

/* Entry in a bucket list, i.e., correspending to value with the same key hash value */
typedef struct BUCKET_ENTRY {
	TCHAR *key;
	DWORD keySize;
	LIST_ENTRY item_List;
	DWORD totalDataSize;			//acummulates the total size of associated key data
	DWORD countItems;
	LIST_ENTRY link;
} BUCKET_ENTRY;

/* The HashTable */
typedef struct HASH_TABLE {
	DWORD count;
	DWORD keysCount;
	DWORD totalKeySize;
	DWORD nBuckets;
	LIST_ENTRY *buckets;
	DWORD mask;
	VCMP vCmp;
	REMFUNC remFunc;
} HASH_TABLE, *PHASH_TABLE;



/* HashTable operations and initialization */
	
VOID HashTableInit(PHASH_TABLE htable, VCMP cmpFunc, REMFUNC remFunc, int capacity);
BOOL HashTableAdd(PHASH_TABLE htable, TCHAR *key, VOID *val, int dataSize);
QueryItemResult *HashTableGet(PHASH_TABLE htable, TCHAR *key, DWORD *countItems, DWORD* totalResultSize);
INT HashTableGet2(PHASH_TABLE htable, TCHAR *key, QueryItemResult *buffer, 
				    DWORD *bufferSize, DWORD *countItems);
VOID* HashTableKeyValues(PHASH_TABLE htable, TCHAR *key, DWORD *countItems);
TCHAR *HashTableKeys(PHASH_TABLE htable, DWORD *keysCount);
BOOL HashTableRemove(PHASH_TABLE htable, TCHAR *key, VOID *val);
VOID HashTableClear(PHASH_TABLE htable);
VOID HashTableDestroy(PHASH_TABLE htable);
	