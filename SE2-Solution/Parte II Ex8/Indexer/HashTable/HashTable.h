/*----------------------------------------------------------------------------
   HashTable variant where can coexist multiple entries with the same key.
   Each bucket in the HashTable is a List of entries with the same key hash value.
   Easch entry in the bucket list is a list of entries with the same exact key.

   The Keys are assumed to be strings.
   Values can be of any type, so a value comparation function is given by the HashTable client.

   ISEL, 2011
-------------------------------------------------------------------------------*/

#pragma once

#ifndef HASHTABLE_EXPORTS
#define HASHTABLE_API __declspec(dllimport)
#else
#define HASHTABLE_API __declspec(dllexport)
#endif

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
	LIST_ENTRY item_List;
	DWORD totalDataSize;			//acummulates the total size of associated key data
	DWORD countItems;
	LIST_ENTRY link;
} BUCKET_ENTRY;

/* The HashTable */
typedef struct HASH_TABLE {
	DWORD count;
	DWORD nBuckets;
	LIST_ENTRY *buckets;
	DWORD mask;
	VCMP vCmp;
	REMFUNC remFunc;
} HASH_TABLE, *PHASH_TABLE;


#ifdef __cplusplus
extern "C" {
#endif
/* HashTable operations and initialization */
	
HASHTABLE_API VOID HashTableInit(PHASH_TABLE htable, VCMP cmpFunc, REMFUNC remFunc, int capacity);
HASHTABLE_API BOOL HashTableAdd(PHASH_TABLE htable, TCHAR *key, VOID *val, int dataSize);
HASHTABLE_API QueryItemResult *HashTableGet(PHASH_TABLE htable, TCHAR *key, DWORD *countItems, DWORD* totalResultSize);
HASHTABLE_API INT HashTableGet2(PHASH_TABLE htable, TCHAR *key, QueryItemResult *buffer, 
				    DWORD *bufferSize, DWORD *countItems);

HASHTABLE_API BOOL HashTableRemove(PHASH_TABLE htable, TCHAR *key, VOID *val);
HASHTABLE_API VOID HashTableClear(PHASH_TABLE htable);
HASHTABLE_API VOID HashTableDestroy(PHASH_TABLE htable);

#ifdef __cplusplus
} // extern "C"
#endif	