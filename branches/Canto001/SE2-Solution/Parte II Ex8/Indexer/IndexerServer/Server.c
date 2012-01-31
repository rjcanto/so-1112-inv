#include <tchar.h>
#include <stdio.h>
#include "..\IndexerService\Indexer.h"

int _tmain(int argc, TCHAR *argv[]) {
	HINDEXER hIndexer;

	// Testar se foi indicado o nome no argumento da linha de comando
	if(argc < 2) {
		_tprintf(_T("Utiliza��o:\n\tServer.exe ROOTPATH\n\n"));
		_tprintf(_T("Prima uma tecla para terminar..."));
		getchar();
		return 0;
	}

	_tprintf(_T("A iniciar o servi�o...\n"));
	hIndexer = CreateFileIndexer(argv[1]);
	if(hIndexer == NULL) {
		_tprintf(_T("ERRO: N�o foi possivel criar o servi�o\n"));
	}
  printf("Servico iniciado..\n");
	while(ProcessNextQuery(hIndexer));

	_tprintf(_T("Prima uma tecla para terminar..."));
	getchar();

	if(hIndexer != NULL)
		CloseIndexer(hIndexer);

	_tprintf(_T("Server terminou!\n"));
	return 0;
}