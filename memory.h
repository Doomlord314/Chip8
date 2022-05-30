#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <pthread.h>
#define MEM_SIZE 0xFFF
#define INTERPRETER_SIZE 0x1FF
#define REGISTER_START 0xF00
#define PROGRAM_BEGIN 0x200
#define PROGRAM_END 0xE9F
#define STACK 0xEA0

typedef unsigned char uchar_t;

typedef struct{
    uchar_t V[0x10];
    uint16_t I;
    uint16_t PC; 
    uchar_t SP;
    uint16_t stack[0x10];  
} registers_t;
void init_program(uchar_t* mem, registers_t* rs);
#endif