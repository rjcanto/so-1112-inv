/*------------------------------------------------------------------------------------------
   Simula que de chuva tirando partido das biblioteca de uthreads, 
   acrescentada da função UtSleep, que promove o bloqueio temporizado da thread invocante
--------------------------------------------------------------------------------------------*/
#include <Windows.h>
#include <stdio.h>
#include "..\Include\UThread.h"

#define SIMULATION_TIME 20000
#define PARTICLES_NUMBER 100

#define BACKGROUND_GRAY (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |BACKGROUND_INTENSITY)
#define BLACK 0

void InitConsole();
void ClearScreen(int backColor, int color);
void WriteCharAt(char c, int x, int y);

int MaxX(); 
int MaxY();

/* A implementar na biblioteca uthreads */
DWORD UtSleep(DWORD ms);

BOOL stop;

VOID RainParticle(UT_ARGUMENT arg) {
	int x, y;
	
	int deltaSleep=(rand()%10)*4;
 
	x= rand() % MaxX();
	y= rand() % MaxY();
	 
	while(!stop) {
		WriteCharAt(' ', x, y);
		 
		x = (x+ ((rand()%4)+1) ) % MaxX();
		y = (y+ ((rand()%4)+1) ) % MaxY();
		
		WriteCharAt('0', x, y);
		UtSleep(20);
	}
}

/* determina o tempo da simulação */
VOID Stopper(UT_ARGUMENT arg) {
	UtSleep(SIMULATION_TIME);
	stop=TRUE;
}

int main(int argc, char* argv[])
{
	int i;

	InitConsole();
	ClearScreen(BACKGROUND_GRAY, BLACK);
	 
	UtInit();

	UtCreate(Stopper, (UT_ARGUMENT) SIMULATION_TIME);
	for (i=0; i < PARTICLES_NUMBER; ++i )
		UtCreate(RainParticle, NULL);
	UtRun();
	 
	UtEnd();

	ClearScreen(BACKGROUND_WHITE, BLACK);
	printf("End of Simulation!\n");
	getchar();
	return 0;
}

