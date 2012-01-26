#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include "stdafx.h"

// Use to convert bytes to KB
#define DIV 1024

// Specify the width of the field in which to print the numbers. 
// The asterisk in the format specifier "%*I64d" takes an integer 
// argument and uses it to pad and right justify the number.
#define WIDTH 


int _tmain(int argc, _TCHAR* argv[])
{
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);

	GlobalMemoryStatusEx (&statex);

	_tprintf (TEXT("There is  %*ld percent of memory in use.\n"),
			WIDTH, statex.dwMemoryLoad);
	_tprintf (TEXT("There are %*I64d total Kbytes of physical memory.\n"),
			WIDTH, statex.ullTotalPhys/DIV);
	_tprintf (TEXT("There are %*I64d free Kbytes of physical memory.\n"),
			WIDTH, statex.ullAvailPhys/DIV);
	_tprintf (TEXT("There are %*I64d total Kbytes of paging file.\n"),
			WIDTH, statex.ullTotalPageFile/DIV);
	_tprintf (TEXT("There are %*I64d free Kbytes of paging file.\n"),
			WIDTH, statex.ullAvailPageFile/DIV);
	_tprintf (TEXT("There are %*I64d total Kbytes of virtual memory.\n"),
			WIDTH, statex.ullTotalVirtual/DIV);
	_tprintf (TEXT("There are %*I64d free Kbytes of virtual memory.\n"),
			WIDTH, statex.ullAvailVirtual/DIV);

	// Show the amount of extended memory available.

	_tprintf (TEXT("There are %*I64d free Kbytes of extended memory.\n"),
			WIDTH, statex.ullAvailExtendedVirtual/DIV);
	return 0;
}