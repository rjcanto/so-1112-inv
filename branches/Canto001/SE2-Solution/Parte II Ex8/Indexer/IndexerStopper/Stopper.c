#include <tchar.h>
#include <stdio.h>
#include "..\IndexerService\Indexer.h"

int _tmain(int argc, TCHAR *argv[]) {
	_tprintf(_T("Stop Indexer...\n"));

	while (!StopIndexer())
		_tprintf(_T("ERRO: ao concluir o serviço..."));

	_tprintf(_T("Prima uma tecla para terminar..."));
	getchar();
	return 0;
}