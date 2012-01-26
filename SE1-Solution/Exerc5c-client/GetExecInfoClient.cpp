#include "stdafx.h"
#include "..\Exerc5c\getexecinfodll.h"
#include <time.h>
//#include <WinNT.h>

int _tmain(int argc, _TCHAR* argv[])
{
	PEXEC_INFO info = (PEXEC_INFO) malloc(sizeof(*info));
	unsigned int idxSection = 0;

	if (argc != 2) {
		printf("Usage: GetExecInfo <file>\n");
		return 1;
	}	
	if (GetExecInfo((LPCSTR)argv[1],info) == 0) {
		printf("System error=%d\n", GetLastError());
		exit(1);
	}
	
	switch (info->type) {
		case NATIVE_APP_32:
			printf("Aplicacao nativa a 32 bits.\n");
			break;
		case NATIVE_APP_64:
			printf("Aplicacao nativa a 64 bits.\n");
			break;
		case NATIVE_DLL_32:
			printf("DLL nativa a 32 bits.\n");
			break;
		case NATIVE_DLL_64:
			printf("DLL nativa a 64 bits.\n");
			break;
	}
	
	printf("Data de construcao: %s\n",ctime((const time_t*) &(info->timeStamp)));
	printf("%x Seccoes:\n",info->nSections);

	while(idxSection < info->nSections){
		printf("\tSeccao %s:\n",info->sections[idxSection].name);
		if ((info->type == NATIVE_APP_32) || (info->type == NATIVE_DLL_32))
			printf("\t\tVirtualAddress: %x\n",info->sections[idxSection].baseSectionAddress.LowPart + info->baseAddress.LowPart);
		else printf("\t\tVirtualAddress: %I64x\n",info->sections[idxSection].baseSectionAddress.QuadPart + info->baseAddress.QuadPart);
		printf("\t\tSize: %x\n",info->sections[idxSection].size);
		idxSection++;
	}

	free(info);
	return 0;
}
