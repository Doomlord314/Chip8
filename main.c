#define MEM_SIZE 0xFFF
#define INTERPRETER_SIZE 0x1FF
#define SCREEN_WIDTH 0x50
#define SCREEN_HEIGHT 0x20
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
#include <conio.h>
#include <windows.h>
#include <pthread.h>
unsigned char memory[MEM_SIZE] = {0};
unsigned char screen[SCREEN_WIDTH*SCREEN_HEIGHT];
unsigned char *V;
unsigned char* low_I;
unsigned char* high_I;
uint16_t* I; 
int milliNow = 0;
int milliLast = 0;
char keyboard_map[0x10] = "X123QWEASDCZ4RFV";
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

int pressed(char key){
	return GetAsyncKeyState(keyboard_map[key]);
}
int wait_pressed(){
	char ch;
	while(1){
		if(_kbhit()){
			ch = _getch();
			for(int i = 0; i<0x10; i++){
				if(keyboard_map[i] == ch || keyboard_map[i]+0x20 == ch ){
					return i;
				}
			}
		}
	}
}


unsigned char** display;

/*Manages Memory*/
void setup()
{
	V  = memory+REGISTER_START;
	I  = memory+REGISTER_START+16;
	DT = memory+80;
	ST = memory+81;
	*DT = 30;
	*ST = 30;
	display = malloc(SCREEN_HEIGHT);
	for(int i = 0; i < SCREEN_HEIGHT; i++){
		display[i] = malloc(SCREEN_WIDTH);
		if(display[i] == NULL){
			perror("setup: NPE");
		}
		for(int j = 0; j < SCREEN_WIDTH; j++){
			display[i][j] = 0;
		}
	}
	
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
		if(*DT != 0){
		//printf("HERE");
			*DT = *DT - 1;
		}
		if(*DT == 0){
			//printf("DT NOW == 0!");
		}
		if(*ST != 0){
		//printf("HERE");
			*ST = *ST - 1;
		}
	}
}


void draw(int sprite, unsigned char x, unsigned char y, unsigned char sprite_size) {
	system("cls");
	for(int i = 0; i < sprite_size; i++){
		for(int j = 0; j < 8; j++){
			if(memory[sprite+i]&((int)pow(2, 8-j))){
				V[0xF] = display[y+i][x+j]?1:V[0xF];
				display[y+i][x+j] = !display[y+i][x+j] ;
			}
		}
	}
	for(int i = 0; i < SCREEN_HEIGHT; i++){
		for(int j = 0; j < SCREEN_WIDTH; j++){
			printf("%c", display[i][j]?0xDB:' ');
		}
		printf("\n");
	}
}
void clear() {
	system("cls");
	if(display[12] == NULL){
		printf("AAAAA");
	}

	for(int i = 0; i < SCREEN_HEIGHT; i++){
		if(display[i] == NULL){
			printf("AAAAA %d %d %d %d %d \n", i, display[i-1], display[i-2], display, display[i]);
			display[i] = malloc(SCREEN_WIDTH);
			for(int j = 0; j < SCREEN_WIDTH; j++){
				display[i][j] = 0;
			}
		}

		for(int j = 0; j < SCREEN_WIDTH; j++){
			display[i][j] = 0;
		}
	}
}
void parser() {
	srand(time(NULL));
	int itter = PROGRAM_BEGIN;
	uint16_t* stack = memory+STACK; /* dangerous, yet a risk I am willing to take */
	int stack_itter = 0;
	uint16_t c = memory[itter+1] + (memory[itter]*0x100);
	while(itter < PROGRAM_END){
		
		/* general variables used in the program */
		unsigned char x   = (c-((c/0x1000)*0x1000))/0x100; /* third digit of (c - (last digit)) */
		unsigned char y   = (c-((c/0x100)*0x100))/0x10; /* second digit of (c - (last 2 digits)) */
		unsigned char n   = (c-((c/0x10)*0x10)); /* c - (last 3 digits) */
		unsigned char kk  = c-((c/0x100)*0x100); /*c - (last 2 digits)*/
		unsigned int nnn = c-((c/0x1000)*0x1000); /* c - (last digit) */
		/* do different things depending on the last digit of c*/
		switch((int)(c/0x1000)){
			case 0x0:
				/* clear*/
				if(c == 0x00E0){
					clear();
				}
				/* return from subroutine */
				if(c == 0x00EE){
					if(stack_itter == 0){
						printf("chip8: stack has no values, yet was still asked to return from subroutine\n");
					} else {
						stack_itter-=1;
						itter = stack[stack_itter] ;
					}
				}
			break;
			/* 1nnn: JMP nnn */
			case 0x1:
				itter = nnn;
			break;
			/* 2nnn: JMP SUBROUTINE @nnn */
			case 0x2:
				stack[stack_itter] = itter+2;
				stack_itter++;
				itter = nnn;
			break;
			/* 3xkk: SKIP IF Vx == kk */
			case 0x3:
				if(V[x] == kk){
					itter+=2;
				}				
			break;
			/* 4xkk: SKIP IF Vx != kk */
			case 0x4:
				if(V[x] != kk){
					itter+=2;
				}	
			break;
			/* 5xy0: SKIP IF Vx == Vy*/
			case 0x5:
				if(V[x] == V[y]){
					itter+=2;
				}	
			break;
			/* 6xkk: SET Vx = kk*/
			case 0x6:
				V[x] = kk;	
			break;
			/* 3xkk: SET Vx = Vx + kk*/
			case 0x7:
				V[x] = V[x] + kk;	
			break;
			/* multiple */
			case 0x8:
				switch(n){
					case 0x0:
						V[x] = V[y];	
					break;
					case 0x1:
						V[x] = V[x] | V[y];	
					break;
					case 0x2:
						V[x] = V[x] & V[y];	
					break;
					case 0x3:
						V[x] = V[x] ^ V[y];	
					break;
					case 0x4:
						V[0xf] = (V[x] + V[y] > 255);
						V[x] = (V[x] + V[y])%255;
					break;
					case 0x5:
						V[0xf] = V[x] > V[y];
						V[x] = V[x] - V[y];
					break;
					case 0x6:
						V[0xf] = V[x] | 0x1;
						V[x] >>= 1;
					break;
					case 0x7:
						V[0xf] = V[y] > V[x];
						V[x] = V[y] - V[x];
					break;
					case 0xE:
						V[0xf] = V[x] | 0x80; /*0x80 = b1000 0000*/
						V[x] <<= 1;
					break;
				}
			break;
			case 0x9:
				if(V[x] != V[y]){
					itter+=2;
				}
			break;
			case 0xA:
				*I = nnn;
			break;
			case 0xB:
				itter = nnn+V[0];
			break;
			case 0xC:
				V[x] = kk&(rand()%255);
			break;
			case 0xD:
				draw(*I, V[x], V[y], n);
			break;
			case 0xE:
				if(kk == 0x9E){
					if(pressed(V[x]))
						itter+=2;
				} 
				if(kk == 0xA1){
					if(!pressed(V[x]))
						itter+=2;
				}
			break;
			case 0xF:
				switch(kk){
					case 0x07:
						printf("WAITING %d ", *DT);
						V[x] = *DT;
					break;
					case 0x0A:
						V[x] = wait_pressed();
					break;
					case 0x15:
						*DT = V[x];
					break;
					case 0x18:
						*ST = V[x];
					break;
					case 0x1E:
						*I += V[x];
					break;
					case 0x29:
						*I = V[x]*5;
					break;
					case 0x33:
						memory[(*I)] =  V[x]/100;
						memory[(*I)+1] = (V[x]-((V[x]/100)*100))/10;
						memory[(*I)+2] =  V[x]-((V[x]/10)*10);
					break;
					case 0x55:
						for(int j = 0; j < x; j++){
							memory[(*I)+j] = V[x];
						}
					break;
					case 0x65:
						for(int j = 0; j < x; j++){
							V[x] = memory[(*I)+j];
						}
					break;
				}
			break;
			
		}
		c = memory[itter+1] + (memory[itter]*0x100);
		itter+=2;
	}
	
}

int main(int argc, char ** argv) {
	setup();
	if(argc < 2){
		printf("chip-8: no file!");
		return 1;
	}
	char* filename = argv[1];
	printf("%s\n", filename);
	FILE* binfile = fopen(filename, "rb");
	int temp = 0;
	unsigned char c = 0;
	int i = PROGRAM_BEGIN;
	// repeat until c is EOF
	while(i < PROGRAM_END)
	{
		temp = fgetc((FILE*)binfile);
		if(temp == EOF)
			break;
		c=(unsigned char)temp;
		memory[i] = c;
		i++;
	}
	pthread_t counter;
	int counterEC;
	counterEC = pthread_create(&counter, NULL, timedCounter, (void *)&DT);
	printf("read memory\n");
	parser();
	
}