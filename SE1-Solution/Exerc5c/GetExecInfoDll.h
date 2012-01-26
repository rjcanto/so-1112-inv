#ifdef GETEXECINFODLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API  __declspec(dllimport)
#endif

#ifdef UNICODE
#define LPCSTR LPCWSTR
#endif

#ifdef __cplusplus 
extern "C" {
#endif
typedef enum EXEC_TYPES {
	NATIVE_APP_32, /* Aplica��o nativa a 32 bits */
	NATIVE_APP_64, /* Aplica��o nativa a 64 bits */
	NATIVE_DLL_32, /* Dll nativa a 32 bits */
	NATIVE_DLL_64, /* Dll nativa a 64 bits */
	/* a partir daqui s�o valores opcionais,
	necess�rios apenas para quem lidou com a al�nea b)
	*/
	
	//MANAGED_MODULE, /* M�dulo managed sem c�digo nativo */
	//MANAGED_MODULE_WITH_NATIVE_32, /* M�dulo managed com c�digo nativo a 32 bits */
	//MANAGED_MODULE_WITH_NATIVE_64, /* M�dulo managed com c�digo nativo a 64 bits */
	//MANAGED_APP, /* Aplica��o managed sem c�digo nativo */
	//MANAGED_APP_WITH_NATIVE_32, /* Aplica��o managed com c�digo nativo a 32 bits */
	//MANAGED_APP_WITH_NATIVE_64 /* Aplica��o managed com c�digo nativo a 64 bits */
	
} EXEC_TYPES;

#define MAX_SECTION_NAME 16 /* tamanho m�ximo do nome de uma sec��o */
#define MAX_SECTIONS 32 /* numero m�ximo de sec��es */

/* estrutura que descreve uma sec��o da imagem */
typedef struct SECTION_DESCRIPTOR {
	CHAR name[MAX_SECTION_NAME];
	DWORD size;
	LARGE_INTEGER baseSectionAddress;
} SECTION_DESCRIPTOR, *PSECTION_DESCRIPTOR;

typedef struct EXEC_INFO {
	LARGE_INTEGER baseAddress; /* endere�o base da imagem */
	DWORD timeStamp; /* data da constru��o(n�mero de segundos a partir de 1/1/1970) */
	EXEC_TYPES type; /* tipo de execut�vel */
	DWORD nSections; /* n�mero de sec��es */
	SECTION_DESCRIPTOR sections[MAX_SECTIONS]; /* array de descritores de sec��es */
} EXEC_INFO, *PEXEC_INFO;

/* fun��o que preenche a estrutura �info� com informa��o sobre o excut�vel de nome �fileName�.
A fun��o retorna TRUE em caso de sucesso, FALSE caso contr�rio.
*/

DLL_API
BOOL GetExecInfo(LPCSTR fileName, PEXEC_INFO info);

#ifdef __cplusplus 
}
#endif