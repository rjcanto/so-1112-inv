#include "stdafx.h"
#include "httpserver.h"

typedef struct DictionaryEntry {
	TCHAR *key;
	char *value;
} DictionaryEntry;

static DictionaryEntry contentTypes[] = {
	{_T("doc"),		"application/msword"},
	{_T("docx"),	"application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
	{_T("gif"),		"image/gif"			},
	{_T("jpg"),		"image/jpeg"		},
	{_T("html"),	"text/html"			},
	{_T("txt"),		"text/plain"		},
	{_T("htm"),		"text/html"			},
	{_T("pdf"),		"application/pdf"	},
	{_T("png"),		"image/png"			}
};
#define CONTENT_TABLESIZE	(sizeof(contentTypes)/sizeof(DictionaryEntry))

char *GetContentType(TCHAR *key) {
	int i;
	for(i=0; i < CONTENT_TABLESIZE; ++i) {
		if (!_tcscmp (contentTypes[i].key,key))
			return contentTypes[i].value;
	}
	/* just one default in case the extension is not found */
	return "text/plain";
}


/*
 * Code for header´s lists management
 */
void HeaderListInit(HeaderNode *head) {
	head->next = head;
	head->headerKey = head->headerValue = NULL;
}

/*
 * Insert a new node allocating memory fron node from Pool p
 */
void HeaderInsert(HeaderNode *head, PPool p, char *key, char *value) {
	HeaderNode *nn = (HeaderNode*) PoolAlloc(p, sizeof(HeaderNode));
	nn->headerKey=key;
	nn->headerValue=value;
	nn->next =  head->next;
	head->next = nn;
}

/*
 * Retrieve the value from the given header name in the list with head "head"
 */
char *HeaderGet(HeaderNode *head, char *key) {
	HeaderNode *curr = head->next;

	while (curr != head) {
		if (!strcmp(curr->headerKey,key))
			return curr->headerValue;
		curr = curr->next;
	}
	return NULL;
}