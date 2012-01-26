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
	NATIVE_APP_32, /* Aplicação nativa a 32 bits */
	NATIVE_APP_64, /* Aplicação nativa a 64 bits */
	NATIVE_DLL_32, /* Dll nativa a 32 bits */
	NATIVE_DLL_64, /* Dll nativa a 64 bits */
	/* a partir daqui são valores opcionais,
	necessários apenas para quem lidou com a alínea b)
	*/
	
	//MANAGED_MODULE, /* Módulo managed sem código nativo */
	//MANAGED_MODULE_WITH_NATIVE_32, /* Módulo managed com código nativo a 32 bits */
	//MANAGED_MODULE_WITH_NATIVE_64, /* Módulo managed com código nativo a 64 bits */
	//MANAGED_APP, /* Aplicação managed sem código nativo */
	//MANAGED_APP_WITH_NATIVE_32, /* Aplicação managed com código nativo a 32 bits */
	//MANAGED_APP_WITH_NATIVE_64 /* Aplicação managed com código nativo a 64 bits */
	
} EXEC_TYPES;

#define MAX_SECTION_NAME 16 /* tamanho máximo do nome de uma secção */
#define MAX_SECTIONS 32 /* numero máximo de secções */

/* estrutura que descreve uma secção da imagem */
typedef struct SECTION_DESCRIPTOR {
	CHAR name[MAX_SECTION_NAME];
	DWORD size;
	LARGE_INTEGER baseSectionAddress;
} SECTION_DESCRIPTOR, *PSECTION_DESCRIPTOR;

typedef struct EXEC_INFO {
	LARGE_INTEGER baseAddress; /* endereço base da imagem */
	DWORD timeStamp; /* data da construção(número de segundos a partir de 1/1/1970) */
	EXEC_TYPES type; /* tipo de executável */
	DWORD nSections; /* número de secções */
	SECTION_DESCRIPTOR sections[MAX_SECTIONS]; /* array de descritores de secções */
} EXEC_INFO, *PEXEC_INFO;

/* função que preenche a estrutura “info” com informação sobre o excutável de nome “fileName”.
A função retorna TRUE em caso de sucesso, FALSE caso contrário.
*/

DLL_API
BOOL GetExecInfo(LPCSTR fileName, PEXEC_INFO info);

#ifdef __cplusplus 
}
#endif