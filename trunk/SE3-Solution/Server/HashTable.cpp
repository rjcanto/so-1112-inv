#include "stdafx.h"


#include "hashtable.h"

/* The hash function simply calculates the XOR between key chars */
static DWORD HashFunc(TCHAR *key) {
	DWORD hashVal=0;
	TCHAR c;

	while (c= *key++) hashVal ^= c;
	return hashVal;
}

/*-------------------------------------------------------
  Fills buffer "vals" with a copy of the items with a given key.
  The format used is the sequence <item size: int><item data><item size: int><item data>...<0>
  The returned array must be freed by the caller
  Returns the number of copied bytes.
---------------------------------------------------------*/
static DWORD CopyToArrayAux(BUCKET_ENTRY *bentry, QueryItemResult *vals) {
	INT *currDataPtr;

	currDataPtr= (INT *) vals;
	LIST_ENTRY *curr = bentry->item_List.Flink;
	while (curr != &bentry->item_List) {
		ITEMLIST_ENTRY *ie = CONTAINING_RECORD(curr, ITEMLIST_ENTRY, link);
		int dataSize = ie->dataSize-sizeof(int);
		*currDataPtr=dataSize;// put item size in result buffer
		memcpy(currDataPtr+1, ie->val, dataSize);
		currDataPtr += (ie->dataSize>>2); // to next item place (assuming sizeof(int)=4)
		curr = curr->Flink;
	}
	*currDataPtr=0;
	return bentry->totalDataSize;
}

/*-------------------------------------------------------
  Fills buffer "vals" with a copy of the items with a given key.
  The returned array must be freed by the caller
  Returns the number of copied items.
---------------------------------------------------------*/
VOID * CopyToArray2(BUCKET_ENTRY *bentry) {
	BYTE *currDataPtr, *start;
	
	if ((currDataPtr = (BYTE *) malloc(bentry->totalDataSize))==NULL)
		return NULL;
	start = currDataPtr;
	LIST_ENTRY *curr = bentry->item_List.Flink;
	while (curr != &bentry->item_List) {
		ITEMLIST_ENTRY *ie = CONTAINING_RECORD(curr, ITEMLIST_ENTRY, link);
		int dataSize = ie->dataSize-sizeof(int);
		memcpy(currDataPtr, ie->val, dataSize);
		currDataPtr += dataSize; // to next item place  
		curr = curr->Flink;
	}
	return start;
}


/*-------------------------------------------------------
  Return a copy of the items with a given key.
  The format used is the sequence <item size: int><item data><item size: int><item data>...<0>
  The returned array must be freed by the caller.
---------------------------------------------------------*/
static QueryItemResult *CopyToArray(BUCKET_ENTRY *bentry) {
	QueryItemResult *vals;

	if ((vals = (QueryItemResult *) malloc(bentry->totalDataSize+sizeof(int)))==NULL)
		return NULL;
	
	CopyToArrayAux(bentry,vals);
	return vals;
}

/* Auxiliary method to find a key in a bucket */
static BUCKET_ENTRY *FindInBucket(LIST_ENTRY *blist, TCHAR *key) {
	LIST_ENTRY *curr = blist->Flink;
	while (curr != blist) {
		BUCKET_ENTRY *bi = CONTAINING_RECORD(curr, BUCKET_ENTRY, link);
		if (!_tcscmp(bi->key, key))
			return bi;
		curr = curr->Flink;
	}
	return NULL;
}

static VOID *RemoveEntryOnList(BUCKET_ENTRY *bentry, VOID *val, VCMP cmpFunc) {
	LIST_ENTRY *curr = bentry->item_List.Flink;
	while (curr != &bentry->item_List) {
		ITEMLIST_ENTRY *ie = CONTAINING_RECORD(curr, ITEMLIST_ENTRY, link);
		if (!cmpFunc(ie->val, val)) {
			VOID *itemData = ie->val;
			RemoveEntryList(curr);
			bentry->countItems--;
			free(ie);
			return itemData;
		}
		curr = curr->Flink;
	}
	return NULL;
}



/*----------------------------------------------------------------------
 * Auxiliary method to remove an entry in a bucket 
 * Output parameter "last" is set to TRUE in case it was the last key item
----------------------------------------------------------------------*/
static VOID* RemoveInBucket(LIST_ENTRY *blist, TCHAR *key, VOID *val, BOOL *last, VCMP cmpFunc) {
	BUCKET_ENTRY *bi = FindInBucket(blist,key);
	VOID *itemData;

	if (bi==NULL) return NULL;
	if ((itemData=RemoveEntryOnList(bi, val,cmpFunc))==NULL) 
		return NULL;
	if (IsListEmpty(&bi->item_List)) {
		RemoveEntryList(&bi->link);
		free(bi);
		*last=TRUE;
	}
	else {
		*last=FALSE;
	}
	return itemData;
}

/*----------------------------------------------------------------------
 * Add a new key,value pair to the bucket
 * returns:
 *	 0 in case of error
 *	 1 if it is the first item for the key
 *	 2 if the key already exists
----------------------------------------------------------------------*/
static INT AddToBucket(LIST_ENTRY *blist, TCHAR *key, VOID *val, int dataSize) {
	BUCKET_ENTRY *bi;
	ITEMLIST_ENTRY *ie;
	INT res =2;

	if ((ie = (ITEMLIST_ENTRY*) malloc(sizeof(ITEMLIST_ENTRY))) == NULL)
		return 0;
	bi = FindInBucket(blist, key);
	if (bi==NULL) {
		if ((bi = (BUCKET_ENTRY *) malloc(sizeof(BUCKET_ENTRY))) == NULL) {
			if (ie != NULL) free(ie);
			return FALSE;
		}
		
		bi->totalDataSize=0;
		bi->key = _tcsdup(key); // duplicate key string so it is owned by the hash table
		bi->countItems=0;
		bi->keySize=_tcslen(key);
		InitializeListHead(&bi->item_List);
		InsertTailList(blist, &bi->link);
		res = 1;
	}
	// align data size to integer multiple due to memory alignments
	ie->dataSize = ((dataSize-1)/sizeof(int) +2)*sizeof(int);
	ie->val=val;
	bi->totalDataSize += ie->dataSize;
	InsertTailList(&bi->item_List, &ie->link);
	bi->countItems++;
	return res;
}

/*---------------------------------------------------------------------
	Public interface
-----------------------------------------------------------------------*/
	
VOID HashTableInit(PHASH_TABLE htable, VCMP cmpFunc, REMFUNC remFunc,int capacity) {
	DWORD nb = capacity== 0 ? 1024 : capacity/2;
	DWORD i=2;
	DWORD mask=1;

	while( i < nb) {
		i*=2;
		mask = (mask << 1) + 1;
	}
	nb=i;
	htable->nBuckets = nb;
	htable->buckets = (LIST_ENTRY*) malloc(sizeof(LIST_ENTRY)*nb);
	for(i=0; i < nb; ++i)
		InitializeListHead(htable->buckets+i);
	htable->count=0;
	htable->keysCount=0;
	htable->totalKeySize=0;
	htable->mask=mask;
	htable->vCmp=cmpFunc;
	htable->remFunc = remFunc;
}

BOOL HashTableAdd(PHASH_TABLE htable, TCHAR *key, VOID *val, int dataSize) {
	DWORD bucketIndex= HashFunc(key) & htable->mask;
	INT res;
	

	if ((res= AddToBucket(htable->buckets + bucketIndex, key, val,dataSize)) ==  0)
		return FALSE;
	htable->count++;
	if (res ==1) {
		htable->keysCount++;
		htable->totalKeySize+= (_tcslen(key)+1)*2;
	}
	return TRUE;
}

QueryItemResult* HashTableGet(PHASH_TABLE htable, TCHAR *key, DWORD *countItems, DWORD* totalResultSize) {
	DWORD bucketIndex= HashFunc(key) & htable->mask;
	BUCKET_ENTRY *bi = FindInBucket(htable->buckets + bucketIndex, key);
	if (bi==NULL) return NULL;
	if (countItems != NULL)
		*countItems = bi->countItems;
	if (totalResultSize != NULL)
		*totalResultSize = bi->totalDataSize;
	return CopyToArray(bi);
}

/*-----------------------------------------------------------------------------------
 * Returns simply the  array of items   with the given key.
 * Used when all items are the same size.
 * The returned array mus be freed by caller.
 *----------------------------------------------------------------------------------*/
VOID* HashTableKeyValues(PHASH_TABLE htable, TCHAR *key, DWORD *countItems) {
	DWORD bucketIndex= HashFunc(key) & htable->mask;
	BUCKET_ENTRY *bi = FindInBucket(htable->buckets + bucketIndex, key);
	
	if (bi==NULL) {
		if (countItems != NULL)
			*countItems = 0;
		return NULL;
	}
	if (countItems != NULL)
		*countItems = bi->countItems;
	return CopyToArray2(bi);
}

/*----------------------------------------------------
	A get version where result buffer is allocated by client. It´s used the following semantic:
	On input, "bufferSize" parameter points to an integer with the size (in bytes) of the client buffer
	(parameter "buffer").
	If the given key isn´t found, it returns -1.
	If "bufferSize" isn´t sufficient to acommodate result, the function returns 0 and
	put the necessary buffer size at the integer pointed by "bufferSize". 
	Note the value can´t be completely accurate, since the HashTable can be changed before next try.

	If "bufferSize" is sufficient, the function puts the result in the buffer pointed by "buffer" and returns
	the used size.
-----------------------------------------------------*/ 
INT HashTableGet2(PHASH_TABLE htable, TCHAR *key, QueryItemResult *buffer, DWORD *bufferSize, DWORD *countItems) {
	DWORD bucketIndex= HashFunc(key) & htable->mask;
	BUCKET_ENTRY *bi = FindInBucket(htable->buckets + bucketIndex, key);
	if (bi==NULL) 
		return -1;
	if (bufferSize == NULL || *bufferSize < bi->totalDataSize) {
		*bufferSize=bi->totalDataSize;
		return 0;
	}
	*countItems = bi->countItems;
	return CopyToArrayAux(bi, buffer);
}

BOOL HashTableRemove(PHASH_TABLE htable, TCHAR *key, VOID *val) {
	DWORD bucketIndex= HashFunc(key) & htable->mask;
	VOID *itemData;
	BOOL last;

	if  ((itemData=RemoveInBucket(htable->buckets + bucketIndex, key, val, &last, htable->vCmp))==NULL)
	 return FALSE;
	htable->count--;
	if (last) {
		htable->keysCount--;
		htable->totalKeySize -= (_tcslen(key)+1)*2;
	}
	// invoke remove callback if it exists
	if (htable->remFunc != NULL)
		htable->remFunc(itemData);
	return TRUE;
}

// empty hash table contents
VOID HashTableClear(PHASH_TABLE htable) {
	DWORD i;

	for (i=0; i < htable->nBuckets; ++i) {
		LIST_ENTRY *currBucketItem = htable->buckets[i].Flink, *nextBucketItem;
		while (currBucketItem != htable->buckets+i) {
			BUCKET_ENTRY *be = CONTAINING_RECORD(currBucketItem, BUCKET_ENTRY, link);
			LIST_ENTRY *currItem = be->item_List.Flink, *nextItem;
			while (currItem != &be->item_List) {
				ITEMLIST_ENTRY *ie = CONTAINING_RECORD(currItem, ITEMLIST_ENTRY, link); 
				if (htable->remFunc != NULL)
					htable->remFunc(ie->val);
				nextItem= currItem->Flink;
				free(ie);
				currItem=nextItem;
			}
			free(be->key);
			nextBucketItem = currBucketItem->Flink;
			free(be);
			currBucketItem = nextBucketItem;
		}
		InitializeListHead(htable->buckets+i);
	} 
}

// destroy hash table 
VOID HashTableDestroy(PHASH_TABLE htable) {
	HashTableClear(htable);
	free(htable->buckets);
	ZeroMemory(htable, sizeof(htable));
}

/*-------------------------------------------------------------------------------------
 * Return the sequence of keys (terminated by an empty key) presented in the hash table
 *------------------------------------------------------------------------------------*/
TCHAR *HashTableKeys(PHASH_TABLE htable, DWORD *keysCount) {
	TCHAR *vals;
	TCHAR *curr;
	DWORD i, currCount;

	currCount = htable->keysCount;
	*keysCount = currCount;
	if ((vals = (TCHAR *) malloc(htable->totalKeySize + 2)) == NULL) 
		return NULL;
	curr=vals;
	for (i=0; i < htable->nBuckets && currCount> 0; ++i) {
		LIST_ENTRY *currBucketItem = htable->buckets[i].Flink;
		while (currBucketItem != htable->buckets+i) {
			BUCKET_ENTRY *be = CONTAINING_RECORD(currBucketItem, BUCKET_ENTRY, link);
			
			_tcscpy(curr, be->key);
			curr += be->keySize+1;
			currBucketItem = currBucketItem->Flink;
			currCount--;		 
		}		
	}
	*curr=0;
	return vals;
}
