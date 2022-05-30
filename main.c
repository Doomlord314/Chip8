#include "memory.h"
#include <curses.h>
#define SCREEN_WIDTH 0x50
#define SCREEN_HEIGHT 0x20
#define CHARACTER_SIZE 5

uchar_t* mem;
registers_t rs;

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


int pressed(char key){
	return GetAsyncKeyState(keyboard_map[key]);
}

int wait_pressed(){
	char ch;
	while(1){
		if(ch = getch()){
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
	display = malloc(SCREEN_HEIGHT*sizeof(uchar_t*));
	for(int i = 0; i < SCREEN_HEIGHT; i++){
		display[i] = malloc(SCREEN_WIDTH);
		for(int j = 0; j < SCREEN_WIDTH; j++){
			display[i][j] = 0;
		}
	}

	mem = malloc(MEM_SIZE);
	init_program(mem, &rs);
}
// Counter to be used in thread
void *timedCounter(void * counter_){
	unsigned char *counter;
	counter = (unsigned char *)counter_;
	while(!0){
		Sleep(1000/60);
		if(*DT != 0){
		 	*DT = *DT - 1;
		}
		if(*DT == 0){
			//printw("DT NOW == 0!");
		}
		if(*ST != 0){
		//printw("HERE");
			*ST = *ST - 1;
		}
	}
}


void draw(int sprite, unsigned char x, unsigned char y, unsigned char sprite_size) {
	clear();
	move(y, x);
	for(int i = 0; i < sprite_size; i++){
		for(int j = 0; j < 8; j++){
			if(mem[sprite+i]&((int)pow(2, 8-j))){
				rs.V[0xF] = display[y+i][x+j]?1:rs.V[0xF];
				display[y+i][x+j] = !display[y+i][x+j] ;
			}
		}
	}
	for(int i = 0; i < SCREEN_HEIGHT; i++){
		for(int j = 0; j < SCREEN_WIDTH; j++){
			printw("%c", display[i][j]?0xDB:' ');
		}
		printw("\n");
	}
	refresh();
}
void clear_disp() {
	clear();

	for(int i = 0; i < SCREEN_HEIGHT; i++){
		for(int j = 0; j < SCREEN_WIDTH; j++){
			display[i][j] = 0;
		}
	}
}
void parser() {
	srand(time(NULL));
	rs.PC = PROGRAM_BEGIN;
	rs.SP = 0;
	uint16_t c = mem[rs.PC+1] + (mem[rs.PC]*0x100);
	while(rs.PC < PROGRAM_END){
		move(0,0);	
		printw("loop %x %x\n", rs.PC, c);
		refresh();
		/* general variables used in the program */
		unsigned char x   = (c-((c/0x1000)*0x1000))/0x100; /* third digit of (c - (last digit)) */
		unsigned char y   = (c-((c/0x100)*0x100))/0x10; /* second digit of (c - (last 2 digits)) */
		unsigned char n   = (c-((c/0x10)*0x10)); /* c - (last 3 digits) */
		unsigned char kk  = c-((c/0x100)*0x100); /*c - (last 2 digits)*/
		unsigned int nnn = c-((c/0x1000)*0x1000); /* c - (last digit) */
		/* do different things depending on the last digit of c*/
		switch((int)(c/0x1000)){
			case 0x0:
				if (c == 0x00E0){
						clear_disp();
				}
				if(c == 0x00EE){
					if(rs.SP == 0){
						printf("chip8: stack has no values, yet was still asked to return from subroutine\n");
					} else {
						rs.SP-=1;
						rs.PC = rs.stack[rs.SP] ;
					}
				}
			break;
			/* 1nnn: JMP nnn */
			case 0x1:
				rs.PC = nnn-2;
			break;
			/* 2nnn: JMP SUBROUTINE @nnn */
			case 0x2:
				rs.stack[rs.SP] = rs.PC;
				rs.SP += 1;
				rs.PC = nnn-2;
			break;
			/* 3xkk: SKIP IF Vx == kk */
			case 0x3:
				if(rs.V[x] == kk){
					rs.PC += 2;
				}
			break;
			/* 4xkk: SKIP IF Vx != kk */
			case 0x4:
				if(rs.V[x] != kk){
					rs.PC += 2;
				}
			break;
			/* 5xy0: SKIP IF Vx == Vy*/
			case 0x5:
				if(rs.V[x] == rs.V[y]){
					rs.PC += 2;
				}
			break;
			/* 6xkk: SET Vx = kk*/
			case 0x6:
				rs.V[x] = kk;
			break;
			/* 3xkk: SET Vx = Vx + kk*/
			case 0x7:
				rs.V[x] += kk;
			break;
			/* multiple */
			case 0x8:
				switch(n){
					case 0:;
						rs.V[x] = rs.V[y];
					break;
					case 1:;
						rs.V[x] = rs.V[x] | rs.V[y];
					break;
					case 2:;
						rs.V[x] = rs.V[x] & rs.V[y];
					break;
					case 3:;
						rs.V[x] = rs.V[x] ^ rs.V[y];
					break;
					case 4:;
						rs.V[0xF] = 0;
						if(rs.V[x] + rs.V[y] > 255){
							rs.V[0xF] = 1;
						}
						rs.V[x] = rs.V[x] + rs.V[y];
					break;
					case 5:;
						rs.V[0xF] = 0;
						if(rs.V[x] - rs.V[y] > 0){
							rs.V[0xF] = 1;
						}
						rs.V[x] = rs.V[x] - rs.V[y];
					break;
					case 6:;
						rs.V[x] >>= 1;
					break;
					case 7:;
						rs.V[0xF] = 0;
						if(rs.V[x] - rs.V[y] < 0){
							rs.V[0xF] = 1;
						}
						rs.V[x] = rs.V[x] - rs.V[y];
					break;
					case 0xE:;
						rs.V[x] <<= 1;
					break;
				}
			break;
			/* 9xy0: Skip if Vx != Vy */
			case 0x9:
				if(rs.V[x] != rs.V[y]){
					rs.PC+=2;
				}
			break;
			/* Annn: Set I = nnn */
			case 0xA:
				rs.I = nnn;
			break;
			/* Bnnn: Set program counter to nnn+V0 */
			case 0xB:
				rs.PC = nnn + rs.V[0];
			break;
			/* Cxkk: Set Vx = kk AND a random number */
			case 0xC:
				rs.V[x] = kk & (rand()&0xFF);
			break;
			/* Dxyn: Draw at (x, y) for sprite held at I with size n */
			case 0xD:
				printw("%d, %d, %d, %d\n", rs.I, rs.V[x], rs.V[y], n);
				refresh();
				draw(rs.I, rs.V[x], rs.V[y], n);
			break;
			case 0xE:
				/* Ex9E: if key at Vx is pressed skipped */
				if(kk == 0x9E){
					if(pressed(rs.V[x])){
						rs.PC+=2;
					}
				} 
				/* Ex9E: if key at Vx is not pressed skipped */
				if(kk == 0xA1){
					if(!pressed(rs.V[x])){
						rs.PC+=2;
					}
				}
			break;
			/* Fxkk: multiple*/
			case 0xF:
				switch(kk){
					/* Fx07L set Vx to current DT */
					case 0x07:
						rs.V[x] = *DT;
					break;
					/* Fx0A: wait until a key is pressed, store it in Vx */
					case 0x0A:
						rs.V[x] = wait_pressed();
					break;
					case 0x15:
						*DT = rs.V[x];
					break;
					/* Fx18: Set ST to Vx */
					case 0x18:
						*ST = rs.V[x];
					break;
					/* Fx1E: Add Vx to I */
					case 0x1E:
					
						rs.I = rs.I + rs.V[x];
					break;
					/* Fx29: Set I to the location of the sprite of the digit of Vx */
					case 0x29:;
						rs.I = rs.V[x]*5;
					break;
					/* Fx33: store Vx in decimal notation, starting at I*/
					case 0x33:
						mem[(rs.I)] =  rs.V[x]/100;
						mem[(rs.I)+1] = (rs.V[x]-((rs.V[x]/100)*100))/10;
						mem[(rs.I)+2] =  rs.V[x]-((rs.V[x]/10)*10);
					break;
					/* Fx55: Store registers V0 through Vx in memory starting at location I. */
					case 0x55:
						for(int i = 0; i < x; i++){
							mem[rs.I+i] = rs.V[i]; 
						}
					break;
					/* Fx65: Read registers V0 through Vx from memory starting at location I. */
					case 0x65:
						for(int i = 0; i < x; i++){
							rs.V[i] = mem[rs.I+i]; 
						}
					break;
				}
			break;
		}
		rs.PC+=2;
		c = mem[rs.PC+1] + (mem[rs.PC]*0x100);
	}
	
}

int main(int argc, char ** argv) {
	if(argc == 1){
		printf("No file given!\n");
		exit(1);
	}

	setup();

	uchar_t data[0xDFF] = {0};
	FILE* fp = fopen(argv[1], "rb");
	
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	int size = fread(data, fsize, 1, fp);

	for(int i = 0; i < fsize; i++){
		mem[PROGRAM_BEGIN+i] = data[i];
	}
	printf("File read %d\n", fsize);

	DT = malloc(sizeof(uchar_t));
	ST = malloc(sizeof(uchar_t));

	pthread_t counter;
	int counterEC;
	counterEC = pthread_create(&counter, NULL, timedCounter, (void *)&DT);
	
	initscr();
	printw("file read %d %d\n", fsize, size);
	refresh();
	noecho();
	raw();
	init_console_functions();
	parser();
	endwin();	
}