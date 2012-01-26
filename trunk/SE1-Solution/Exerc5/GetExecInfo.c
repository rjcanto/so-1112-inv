#include "windows.h"
#include <WinNT.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

/*
#ifdef _WIN64
#define IMAGE_OPTIONAL_HEADER IMAGE_OPTIONAL_HEADER64
#endif
*/
#define OPT_HEADER32_IMGBASE_OFFSET 28
#define OPT_HEADER64_IMGBASE_OFFSET 24

int GetExecInfo(char* execFile) {

	HANDLE hExec=INVALID_HANDLE_VALUE;
	HANDLE MapExec=NULL;
	LPVOID fileMapAddress=NULL;
	ULONGLONG imagebase;
	char* execType;

	PIMAGE_DOS_HEADER dosHeader ;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_NT_HEADERS64 pNTHeader64;
	PIMAGE_SECTION_HEADER section ;

	int error = 0, nSections = 0, is32bit=-1, idxSection = 0;

	/*Abertura do ficheiro*/
	hExec = CreateFile(execFile, 
				 GENERIC_READ ,
				 FILE_SHARE_READ,
				 NULL,
				 OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL,
				 NULL);
	if (hExec == INVALID_HANDLE_VALUE) 
		return -1;
	
	/*Criacao do mapa do ficheiro*/
	if ((MapExec = CreateFileMapping(hExec,
				        NULL, 
						PAGE_READONLY,
						0,
						0,
						NULL)) == NULL) {
		error= -4;
		goto Terminate;
	}

	/*Abertura do mapa do ficheiro*/
	if ((fileMapAddress = (LPBYTE) MapViewOfFile(MapExec, 
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
	
	
	/*Verifica se o ficheiro é um executável ou DLL*/
	if((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL)
		execType = "DLL";
	else if ((pNTHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == IMAGE_FILE_EXECUTABLE_IMAGE) 
		execType = "Aplicacao";
	else {
		error=-8;
		goto Terminate;
	}
	
	/* É 32 bits? */
	if((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR32_MAGIC) == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
		printf("%s nativa a 32 bits.\n", execType);
		is32bit=1;
	}/*Ou será a 64?*/
	else if ((pNTHeader->OptionalHeader.Magic & IMAGE_NT_OPTIONAL_HDR64_MAGIC) == IMAGE_NT_OPTIONAL_HDR64_MAGIC){
		printf("%s nativa a 64 bits.\n", execType);
		is32bit=0;
	}

	/*Converte a data de construcao de segundos para data */
	printf("Data de construcao: %s",ctime((const time_t*) &pNTHeader->FileHeader.TimeDateStamp));
	
	nSections = pNTHeader->FileHeader.NumberOfSections;
	printf("%x Seccoes:\n",nSections);
	section = IMAGE_FIRST_SECTION(pNTHeader);

	/*A imagebase para 64bits é diferente que para 32 bits*/
	if (is32bit)
		imagebase = pNTHeader->OptionalHeader.ImageBase;
	else {
		pNTHeader64 = (PIMAGE_NT_HEADERS64) ((BYTE*) dosHeader + ((dosHeader)->e_lfanew));	
		imagebase = pNTHeader64->OptionalHeader.ImageBase;
	}

	/*Percorrer cada seccao e imprimir a sua informacao*/
	while(idxSection < nSections){
		printf("\tSeccao %s:\n",section->Name);
		if (is32bit)
			printf("\t\tVirtualAddress: %x\n",imagebase + section->VirtualAddress);
		else printf("\t\tVirtualAddress: %I64x\n",imagebase + section->VirtualAddress);
		printf("\t\tSize: %x\n",section->SizeOfRawData);
		idxSection++;
		section++;
	}

	/* É necessário limpar os recusos usados */
Terminate:
	if (fileMapAddress!=NULL)	UnmapViewOfFile(fileMapAddress);
	if (hExec != INVALID_HANDLE_VALUE) CloseHandle(hExec);
	return error;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res;
	if (argc != 2) {
		printf("Usage: GetExecInfo <file>\n");
		return 1;
	}	
	if ((res=GetExecInfo(argv[1])) < 0) {
		printf("error %d. System error=%d\n", res, GetLastError());
		exit(1);
	}
	return 0;
}
