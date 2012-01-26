// Exerc6.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef UNICODE
#define CreateFile CreateFileW
#define LPCSTR LPCWSTR
#else
#define CreateFile CreateFileA
#endif

int GetInfo() {
	HANDLE currModule = INVALID_HANDLE_VALUE;
	HANDLE mapExec= INVALID_HANDLE_VALUE;
	ULONGLONG imagebase;
	LPVOID fileMapAddress=NULL;
	PIMAGE_DOS_HEADER dosHeader ;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_NT_HEADERS64 pNTHeader64;
	PIMAGE_SECTION_HEADER section ;
	int error = 0, nSections = 0, idxSection= 0, is32bit=0;

	if ((currModule = (HANDLE) GetModuleHandle(NULL))== INVALID_HANDLE_VALUE)
		return -1;

	if ((mapExec = CreateFileMapping(currModule,
				        NULL, 
						PAGE_READONLY,
						0,
						0,
						NULL)) == INVALID_HANDLE_VALUE) {
		error= -4;
		goto Terminate;
	}

	/*Abertura do mapa do ficheiro*/
	if ((fileMapAddress = (LPVOID) MapViewOfFile(mapExec, 
						FILE_MAP_READ,
						0,
						0,
						0)) == NULL) {
		error= -7;
		goto Terminate;
	}

	dosHeader = (PIMAGE_DOS_HEADER) fileMapAddress;

	/*NT Header é igual ao endereço do DOS Header com o offset de lfanew*/
	pNTHeader = (PIMAGE_NT_HEADERS) ((BYTE*) dosHeader + ((dosHeader)->e_lfanew));


	/* É 32 bits? */
	if((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR32_MAGIC) == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		is32bit=1;
	/*Ou será a 64?*/
	else if ((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR64_MAGIC) == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		is32bit=0;
	
	/*A imagebase para 64bits é diferente que para 32 bits*/
	if (is32bit)
		imagebase = pNTHeader->OptionalHeader.ImageBase;
	else {
		pNTHeader64 = (PIMAGE_NT_HEADERS64) ((BYTE*) dosHeader + ((dosHeader)->e_lfanew));	
		imagebase = pNTHeader64->OptionalHeader.ImageBase;
	}

	nSections = pNTHeader->FileHeader.NumberOfSections;
	section = IMAGE_FIRST_SECTION(pNTHeader);

	while(idxSection < nSections){
		if (strcmp((CHAR*)section->Name,".text")) {
			if (is32bit)
				printf("\t\tVirtualAddress: %x\n",imagebase + section->VirtualAddress);
			else printf("\t\tVirtualAddress: %I64x\n",imagebase + section->VirtualAddress);
		}
		idxSection++;
		section++;
	}
	
Terminate:
	if (fileMapAddress!=NULL)	UnmapViewOfFile(fileMapAddress);
	if (mapExec != INVALID_HANDLE_VALUE) CloseHandle(mapExec);
	return error;
}
int _tmain(int argc, _TCHAR* argv[])
{
	int res;
	
	if ((res=GetInfo()) < 0) {
		printf("error %d. System error=%d\n", res, GetLastError());
		getchar();
		exit(1);
	}
	getchar();
	return 0;
}

