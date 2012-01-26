#include <windows.h>

static CONSOLE_SCREEN_BUFFER_INFO cinfo;
static HANDLE console;
static int bkColor, fgColor;


void InitConsole() {
	console	 = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console,&cinfo);
	SetConsoleTextAttribute(console, 0xF0);
}

void ClearScreen(int backColor, int color) {
	int x, y;
	TCHAR line[1024];
	COORD c = {0,0};
	CONSOLE_SCREEN_BUFFER_INFO sinfo;

	bkColor = backColor;
	fgColor = color;

	SetConsoleTextAttribute(console, backColor | color);
	GetConsoleScreenBufferInfo(console,&sinfo);
	for (x=0; x < sinfo.dwSize.X; x++) 
		line[x]= ' ';
	for (y=0; y < sinfo.dwSize.Y; y++) 
		WriteConsole(console,line,sinfo.dwSize.X, NULL,NULL);
	SetConsoleCursorPosition(console,c);
}

void WriteCharAt(char c, int x, int y) {
	CHAR_INFO ci;
	COORD region = {1,1},spos={0,0};
	SMALL_RECT rect;
	
	rect.Top = rect.Bottom = y;
	rect.Left = rect.Right = x;

	ci.Attributes =  bkColor | fgColor;
#ifdef UNICODE 
	ci.Char.UnicodeChar = c;
#else
	ci.Char.AsciiChar =c;
#endif

    WriteConsoleOutput(console, &ci, region, spos, &rect);
}


int MaxX() {
	return cinfo.dwMaximumWindowSize.X;
}

int MaxY() {
	return cinfo.dwMaximumWindowSize.Y;
}