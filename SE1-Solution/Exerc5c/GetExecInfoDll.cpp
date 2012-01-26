#include "stdafx.h"
#include "GetExecInfoDll.h"
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef UNICODE
#define CreateFile CreateFileW
#define LPCSTR LPCWSTR
#else
#define CreateFile CreateFileA
#endif

#define WITH_DLL_MAIN

#ifdef WITH_DLL_MAIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		_tprintf(_T("Starting dllgetexecinfo!\n"));
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		_tprintf(_T("Ending dllgetexecinfo!\n"));
		break;
	}
	return TRUE;
}
#endif

BOOL GetExecInfo(LPCSTR fileName, PEXEC_INFO info){

	HANDLE hExec=INVALID_HANDLE_VALUE;
	HANDLE MapExec=NULL;
	LPVOID fileMapAddress=NULL;

	PIMAGE_DOS_HEADER dosHeader ;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_NT_HEADERS64 pNTHeader64;
	PIMAGE_SECTION_HEADER section ;

	int error = 0, nSections = 0, is32bit=-1;
	unsigned int idxSection = 0;

	hExec = CreateFile(fileName, 
				 GENERIC_READ ,
				 FILE_SHARE_READ,
				 NULL,
				 OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL,
				 NULL);
	if (hExec == INVALID_HANDLE_VALUE) 
		return -1;
	
	if ((MapExec = CreateFileMapping(hExec,
				        NULL, 
						PAGE_READONLY,
						0,
						0,
						NULL)) == NULL) {
		error= -4;
		goto Terminate;
	}

	if ((fileMapAddress = (LPBYTE) MapViewOfFile(MapExec, 
						FILE_MAP_READ,
						0,
						0,
						0)) == NULL) {
		error= -7;
		goto Terminate;
	}

	dosHeader = (PIMAGE_DOS_HEADER) fileMapAddress;
	pNTHeader = (PIMAGE_NT_HEADERS) ((BYTE*) dosHeader + ((dosHeader)->e_lfanew));
	
	if((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR32_MAGIC) == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		is32bit=1;

	else if ((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR64_MAGIC) == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		is32bit=0;

	if((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL) {
		if (is32bit) info->type = NATIVE_DLL_32;
		else info->type = NATIVE_DLL_64;
	}
	else {
		if (is32bit) info->type = NATIVE_APP_32;
		else info->type = NATIVE_APP_64;
	}
	
	if (is32bit)
		info->baseAddress.LowPart = pNTHeader->OptionalHeader.ImageBase;
	else {
		pNTHeader64 = (PIMAGE_NT_HEADERS64) ((BYTE*) dosHeader + ((dosHeader)->e_lfanew));	
		info->baseAddress.QuadPart = pNTHeader64->OptionalHeader.ImageBase;
	}
	info->timeStamp = pNTHeader->FileHeader.TimeDateStamp;
	info->nSections = pNTHeader->FileHeader.NumberOfSections;
	
	section = IMAGE_FIRST_SECTION(pNTHeader);
	
	
	while(idxSection < info->nSections){
		strcpy(info->sections[idxSection].name,(CHAR*)section->Name);
		info->sections[idxSection].baseSectionAddress.LowPart = section->VirtualAddress;
		/* É necessário garantir que a High part não tem lixo*/
		info->sections[idxSection].baseSectionAddress.HighPart = 0x00000000 ;
		info->sections[idxSection].size = section->SizeOfRawData;
		idxSection++;
		section++;
	}
	
Terminate:
	if (fileMapAddress!=NULL)	UnmapViewOfFile(fileMapAddress);
	if (hExec != INVALID_HANDLE_VALUE) CloseHandle(hExec);
	return 1;
}
