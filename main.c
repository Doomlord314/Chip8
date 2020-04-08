#define MEM_SIZE 0xFFF
#define INTERPRETER_SIZE 0x1FF
#define SCREEN_WIDTH 0x42
#define SCREEN_HEIGHT 0x21
#define REGISTER_START 0xF00
#define CHARACTER_SIZE 5
#define PROGRAM_BEGIN 0x200
#define PROGRAM_END 0xE9F
#define STACK 0xEA0
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <pthread.h>
unsigned char memory[MEM_SIZE] = {0};
unsigned char screen[SCREEN_WIDTH*SCREEN_HEIGHT];
unsigned char *V;
unsigned char* low_I;
unsigned char* high_I;
uint16_t I; 
int milliNow = 0;
int milliLast = 0;
char keyboard_map[0x10] = "X123QWEASDC4RFV";
unsigned char * ST;
unsigned char * DT;
/*for display on windows*/
static HANDLE hStdout;
static HANDLE hStdin;
static CONSOLE_SCREEN_BUFFER_INFO csbi;
static const COORD startCoords = {0,0};


void init_console_functions (void)
{
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  hStdin = GetStdHandle(STD_INPUT_HANDLE);

  GetConsoleScreenBufferInfo(hStdout, &csbi);
}

void gotoxy(int x, int y)
{
  COORD coord;

  coord.X = x;
  coord.Y = y;

  SetConsoleCursorPosition(hStdout,coord);
}

/*Manages Memory*/
void setup()
{
	V  = memory+REGISTER_START;
	low_I = memory+REGISTER_START+17;
	high_I = memory+REGISTER_START+16;
	DT = memory+80;
	ST = memory+81;
	*DT = 30;
	*ST = 30;
	
	//0
	memory[0] = 0xF0;memory[1] = 0x90;memory[2] = 0x90;memory[3] = 0x90;memory[4] = 0xF0;
	//1
	memory[5] = 0x20;memory[6] = 0x60;memory[7] = 0x20;memory[8] = 0x20;memory[9] = 0x70;
	//2
	memory[10] = 0xF0;memory[11] = 0x10;memory[12] = 0xF0;memory[13] = 0x80;memory[14] = 0xF0;
	//3
	memory[15] = 0xF0;memory[16] = 0x10;memory[17] = 0xF0;memory[18] = 0x10;memory[19] = 0xF0;
	//4
	memory[20] = 0x90;memory[21] = 0x90;memory[22] = 0xF0;memory[23] = 0x10;memory[24] = 0x10;
	//5
	memory[25] = 0xF0;memory[26] = 0x80;memory[27] = 0xF0;memory[28] = 0x10;memory[29] = 0xF0;
	//6
	memory[30] = 0xF0;memory[31] = 0x80;memory[32] = 0xF0;memory[33] = 0x90;memory[34] = 0xF0;
	//7
	memory[35] = 0xF0;memory[36] = 0x10;memory[37] = 0x20;memory[38] = 0x40;memory[39] = 0x40;
	//8
	memory[40] = 0xF0;memory[41] = 0x90;memory[42] = 0xF0;memory[43] = 0x90;memory[44] = 0xF0;
	//9
	memory[45] = 0xF0;memory[46] = 0x90;memory[47] = 0xF0;memory[48] = 0x10;memory[49] = 0xF0;
	//A
	memory[50] = 0xF0;memory[51] = 0x90;memory[52] = 0xF0;memory[53] = 0x90;memory[54] = 0x90;
	//B
	memory[55] = 0xE0;memory[56] = 0x90;memory[57] = 0xE0;memory[58] = 0x90;memory[59] = 0xE0;
	//C
	memory[60] = 0xF0;memory[61] = 0x80;memory[62] = 0x80;memory[63] = 0x80;memory[64] = 0xF0;
	//D
	memory[65] = 0xE0;memory[66] = 0x90;memory[67] = 0x90;memory[68] = 0x90;memory[69] = 0xE0;
	//E
	memory[70] = 0xF0;memory[71] = 0x80;memory[72] = 0xF0;memory[73] = 0x80;memory[74] = 0xF0;
	//F
	memory[75] = 0xF0;memory[76] = 0x80;memory[77] = 0xF0;memory[78] = 0x80;memory[79] = 0x80;
}
// Counter to be used in thread
void *timedCounter(void * counter_){
	unsigned char *counter;
	counter = (unsigned char *)counter_;
	while(!0){
		Sleep(1000/60);
		if(*DT>0){
		//printf("HERE");
			*DT = *DT - 1;
		}
		if(*ST>0){
		//printf("HERE");
			*DT = *DT - 1;
		}
	}
}
unsigned char display[(SCREEN_WIDTH+1)*SCREEN_HEIGHT];
void draw(int sprite, unsigned char x, unsigned char y, unsigned char sprite_size)
{
	for(int i = 0; i < sprite_size; i++)
	{
		for(int j = 0; j < 8   ; j++){
			if(memory[sprite+i]&(int)pow(2,8-j))
			{
				screen[x+j+((y+i)*SCREEN_WIDTH)] = !screen[x+j+((y+i)*SCREEN_WIDTH)];
			
			}
		}
	}
	for(int i = 0; i < SCREEN_HEIGHT; i++)
	{
		for(int j = 0; j < SCREEN_WIDTH; j++)
		{
			if(screen[j+(i*SCREEN_WIDTH)])
				display[j+(i*SCREEN_WIDTH)] = 0xDB;
			else
				display[j+(i*SCREEN_WIDTH)] = ' ';
		}
		display[(SCREEN_WIDTH-1)+(i*SCREEN_WIDTH)] = '\n';
	}
	system("cls");
	printf(display);
	//system("pause");
}
void clear()
{
	int i;
	for(i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++)
	{
		screen[i] = 0;
	}
	system("cls");
	for(int i = 0; i < SCREEN_HEIGHT; i++)
	{
		for(int j = 0; j < SCREEN_WIDTH; j++)
		{
			if(screen[j+(i*SCREEN_WIDTH)])
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
}
void parser()
{
	srand(time(NULL));
	uint32_t i;
	unsigned char I = *low_I + ((int)*high_I*0x100);
	int j;
	for(i = PROGRAM_BEGIN; i<PROGRAM_END; i+=2){
		uint16_t c = memory[i]*0x100 + memory[i+1];
		//printf("i: %x, DT: %d, ST: %d, C: %x\n", i, *DT, *ST, c);
		unsigned char x = (c/0x100)-(c/0x1000)*0x10; 
		unsigned char y = (c/0x10)-(c/0x100)*0x10;
		unsigned char kk = c- ((c/0x100)*0x100);
		unsigned char n = c-((c/0x10)*0x10);
		//printf("x: %x, y: %x, kk: %x, n: %x\n", x, y, kk, n);
		//printf("i: %x", i);
		switch((int)(c/0x1000))
		{
			case 0:
				if(c == 0x00E0){
					clear();
				}
				if(c == 0x00EE){
				printf("FOUND");
					
				 	//printf("STACK RETURN: STACK_NUM=%d, new i=%x\n", memory[STACK], 
					//	(memory[0xEA2]));
					i = (memory[STACK+memory[STACK]*2]<<8)+memory[STACK+memory[STACK]*2+1];
					memory[STACK+memory[STACK]] = 0;
					memory[STACK] = memory[STACK]-1;
				 	//printf("STACK RETURN: STACK_NUM=%d, i=%x\n", memory[STACK], i);
				}
			break;
			//1nnn JUMP
			case 1:
				i=(c%0x1000)-0x2;
			break;
			//2nnn GO TO SUBROTUINE
			case 2:	
				memory[STACK]=memory[STACK]+1;
				if(STACK+memory[STACK]*2<0xEFF)
				{
					memory[STACK+memory[STACK]*2]=(i>>8);
					memory[STACK+memory[STACK]*2+1]=(i)-((i>>8)<<8);
				}
				else
				{
					printf("ERR - Stack Overload");
					return;
				}
				//printf("STACK CALL: STACK_NUM=%d, i=%x\n, memory[STACK+memory[STACK]]=%x%x", memory[STACK], i,
				//	memory[STACK+memory[STACK]*2],memory[STACK+memory[STACK]*2+1]);
				//	system("pause");
				i=(c%0x1000)-0x2;
			break;
			//3xkk SKIP NEXT IF Vx == kk
			case 3:	
				//printf("OPERANDS: %x, %x", *(V+((c/0x100)-((c/0x1000)*0x10))), c - (0x100*(c/0x100)));
				if(
				*(V+x)
				==
				kk)
				{
					i+=2;
				}
			break;
			//4xkk SKIP NEXT IF Vx != kk
			case 4:	
				if(
				*(V+x)
				!=
				kk)
				{
					i+=2;
				}
			break;
			//6xy0 SKIP NEXT IF Vx == Vy
			case 5:	
				if(
				*(V+x)
					==
				*(V+y))
				{
					i+=2;
				}
			break;
			//6xkk Vx = kk
			case 6:	
				//printf("HEELOO");
				*(V+x) = kk;
			break;
			//7xkk Vx += kk
			case 7:	
				*(V+x) += kk;
			break;
			//multiple
			case 8:	;
				switch(c-((c/0x10)*0x10))
				{
					case 0:
						*(V+x) = *(V+y);
					break;
					case 1:
						*(V+x) = *(V+y)|*(V+x);
					break;
					case 2:
						*(V+x) = *(V+y)&*(V+x);
					break;
					case 3:
						*(V+x) = *(V+y)^*(V+x);
					break;
					case 4:
						*(V+0xF) = 0;
						if(*(V+y)+*(V+x) > 255)
							*(V+0xF) = 1;
						*(V+x)   = *(V+y)+*(V+x)%255;
					break;
					case 5:
						*(V+0xF) = 0;
						if(*(V+y)  < *(V+x))
							*(V+0xF) = 1;
						*(V+x)   = *(V+x)-*(V+y);
					break;
					case 6:
						*(V+0xF) = 0;
						if((int)V&1==1)
						{
							*(V+0xF) = 1;
						}
						*(V+x) >>= 1;
					break;
					case 7:
						*(V+0xF) = 0;
						if(*(V+x)  < *(V+y))
						{
							*(V+0xF) = 1;
						}
						*(V+x)   = *(V+x)-*(V+y);
					break;
					case 8:
						*(V+0xF) = 0;
						if((int)V&1==1)
						{
							*(V+0xF) = 1;
						}
						*(V+x) <<= 1;
					break;
				}
			break;
			//9xy0: SKIP IF Vx != Vy
			case 9:	
				if(
				*(V+x)
					!=
				*(V+y))
				{
					i+=2;
				}
			break;
			// Annn: SET I = nnn
			case 0xA:
			//	printf("hello, setting I to %x\n", ((memory[i+1]) + memory[i]%0xA0*0x100));
				*low_I = memory[i+1];
				*high_I = memory[i]%0xA0;
			//	printf("REE %d\n", *high_I);
	//			((*low_I)*0x100)+(*high_I);
			break;
			// Bnnn: SET I = nnn + V0
			case 0xB:
				i = c%0xB000 + *V;
			break;
			//Cxkk: SET Vx = kk&<a random number> 
			case 0xC:
				*(V+x)
				=
				kk&(rand()%255);
			break;
			//Dxyn: Display sprite at I at Vx Vy with size n
			case 0xD:
			//printf("VARS: I=%d, X=%d, Y=%d, n=%d", ((*high_I)*0x100)+(*low_I) , *(V+((c/0x100)-( (c/0x1000)*0x10) ) ), 
				//*(V+((c/0x10)-( ( (int)(c/0x100) ) *0x10) ) ),
				//(((c)-((c/0x10)*0x10) ) ) );
				draw(((*high_I)*0x100)+(*low_I) , 
				*(V+x), 
				*(V+y),
				n);
			break;
			// Get key at Vx pressed state
			case 0xE:
				// if pressed, skip
				if(kk == 0x9E)
				{
					if(GetAsyncKeyState(keyboard_map[*(V+x)]) != 0)
					{
						printf("KEY PRESSED: %d", keyboard_map[*(V+x)]);
						i+=2;
					}
				}
				//if not pressed, skip
				if(kk == 0xA1)
				{
					if(GetAsyncKeyState(keyboard_map[*(V+x)]) == 0)
						i+=2;
				}
			break;
			// multiple
			case 0xF:
			;
				I = *low_I + ((int)*high_I*0x100);
				switch(kk)
				{
					case 0x07:
						*(V+x) = *DT;
					break;
					case 0x0A:
						*(V+x)=getchar();
					break;
					case 0x15:
						*DT=*(V+x);
					break;
					case 0x18:
						*ST=*(V+x);
					break;
					case 0x1E:
						I += *(V+x);
						*high_I = I/0x100;
						*low_I = I-((int)*high_I*0x100);
					break;
					case 0x29:
						I = *(V+x)*0x05;
						*high_I = I/0x100;
						*low_I = I-((int)*high_I*0x100);
					break;
					case 0x33:
						*low_I   = (*(V+x)/0x100);
						*(low_I+2) = ((*(V+x)/0x10)-((*(V+x)/0x100)*0x10));
						*(low_I+4) = (*(V+x)-((*(V+x)/0x10)*0x10));
					break;
					case 0x55:;
						I = *low_I + ((int)*high_I*0x100);
						for(j = 0; j <= x; j++)
						{
							memory[I+j] = *(V+j);
						}
					break;
					case 0x65:;
						I = *low_I + ((int)*high_I*0x100);
						for(j = 0; j <= x; j++)
						{
							*(V+j)=memory[I+j];
						}
					break;
				}
			break;
		    default:
				printf("ERR - GOT VALUE: %d", (int)(c/0x1000));
			break;
		}
	}
}

int main(int argc, char ** argv) {
	int i;
	for(i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++)
	{
		screen[i] = 0;
	}
	int temp = 0;
	unsigned char c = 0;
	unsigned char x = 0;
	i = PROGRAM_BEGIN;
	// repeat until c is EOF
	FILE* fp = fopen("Tetris.ch8", "rb");
	while(i < PROGRAM_END)
	{
		temp = fgetc((FILE*)fp);
		if(temp == -1)
			break;
		c=(unsigned char)temp;
		//printf("%x\n", c);
		memory[i] = c;
		i++;
	}
	
	pthread_t DTCounter;
	pthread_t STCounter;
	unsigned char DTEC, STEC;
    DTEC = pthread_create(&DTCounter, NULL, timedCounter, (void *)&DT);
    STEC = pthread_create(&STCounter, NULL, timedCounter, (void *)&ST);
	setup();
	printf("threads created, exit codes: %d, %d\n", DTEC, STEC);
	parser();
}