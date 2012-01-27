#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include "..\IndexerService\Indexer.h"

#define NR_OF_THREADS 4

VOID WINAPI Queries(TCHAR* fileName) {
    HINDEXER hIndexer;
    QUERY_ENUM ansEnum;
    TCHAR* ocurrence;
    int nrOcurrence;

    printf("A iniciar o cliente:%d, FileName: %S...\n", GetCurrentThreadId(), fileName);
    hIndexer = OpenFileIndexer();
    if(hIndexer == NULL) {
        printf("ERRO: Nao foi possivel abrir o cliente %d\n", GetCurrentThreadId());
        return;
    }
  
    printf("Iniciou com sucesso o cliente %d\n", GetCurrentThreadId());

    printf("Cliente %d a efectuar QueryCount ...\n", GetCurrentThreadId());
    nrOcurrence = QueryCount(hIndexer, fileName);
    if(nrOcurrence == -1)
        printf("Cliente: %d : erro a efectuar QueryCount ...\n", GetCurrentThreadId());
    else
    {
        if(nrOcurrence == 0)
            printf("Client: %d Nao possui nenhum ficheiro com esse nome\n", GetCurrentThreadId());
        else
            printf("Cliente: %d File Name: %S, Nr Ocorrencia: %d \n", GetCurrentThreadId(), fileName, nrOcurrence);
        printf("Cliente: %d Terminou QueryCount ...\n", GetCurrentThreadId());
    }


    printf("Cliente: %d a efectuar QueryFolders ...\n", GetCurrentThreadId());

    ansEnum = QueryFolders(hIndexer, fileName);
    if (ansEnum.answers == NULL || ansEnum.getNext == NULL)
         printf("Cliente: %d : erro a efectuar QueryFolders ...\n", GetCurrentThreadId());
    else
    {
        ocurrence = QueryNext(ansEnum);
        while(ocurrence != NULL)
        {
            printf("Caminho da Ocorrencia: %S \n", ocurrence);
            ocurrence = QueryNext(ansEnum);
        }
        printf("Cliente: %d terminou QueryFolders ...\n", GetCurrentThreadId());
    }
    printf("Prima uma tecla para terminar cliente: %d...\n\n", GetCurrentThreadId());
    getchar();
    printf("Cliente: %d terminou!\n", GetCurrentThreadId());

    if(hIndexer != NULL)
        CloseIndexer(hIndexer);
}

int _tmain(int argc, TCHAR *argv[]) {
	  int		i;
    HANDLE threads[NR_OF_THREADS];
	  // Testar se foi indicado o nome no argumento da linha de comando
	  if(argc < 2) {
		    printf("Utilização:\n\tClient.exe FileName1 ... FileName%d \n\n", NR_OF_THREADS);
		    printf("Prima uma tecla para terminar...");
		    getchar();
		    return 0;
	  }

    for(i = 0; i < NR_OF_THREADS; i++)
    {
        threads[i] = (HANDLE)_beginthreadex(NULL
            , 0
            , (void*)&Queries
            , (void*)argv[i+1]
            , CREATE_SUSPENDED
            , NULL
        );
    }
    

    for(i = 0; i < NR_OF_THREADS; i++)
    {
        ResumeThread(threads[i]);
    }


    WaitForMultipleObjects(4, threads, TRUE, INFINITE);
    getchar();
	  return 0;
}