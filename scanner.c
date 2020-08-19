#include<string.h>
#include<stdbool.h>
#include<stdio.h>

#include "scanner.h"

typedef struct {
	char* start;
	int current;
} Scanner;

Scanner scanner;

void init_scanner(char* source) {
	scanner.start = source;
	scanner.current = 0;
}

TokenType scan_token() {
	while(true){
		char ret = scanner.start[scanner.current];
		// printf("Scanner got %c at index %d\n", ret, scanner.current);
		scanner.current++;	
		if(ret == 0x00){
			return Token_EOF;
		} else if(ret == '>') {
			return Token_R; 
		} else if(ret == '<') {
			return Token_L;
		} else if(ret == '+') {
			return Token_Inc;
		} else if(ret == '-') {
			return Token_Dec;
		} else if(ret == '.') {
			return Token_Period;
		} else if(ret == ',') {
			return Token_Comma;
		} else if(ret == '[') {
			return Token_LB;
		} else if(ret == ']') {
			return Token_RB;
		}
	}
}

void jump_forward(){
	int open = 1;
	while(open > 0) {
		TokenType next = scan_token();
		if(next == Token_LB) {
			open++;
		} else if(next == Token_RB) {
			open--;
		}
	}
	// printf("Scanner jumped forward to %d", scanner.current);
}

void jump_back(){
	int closed = 1;
	scanner.current--; // Jump back one extra so we don't start looking on the ']' that caused this call in the first place
	while(closed > 0) {
		scanner.current--;
		char prev = scanner.start[scanner.current];
		if(prev == '[') {
			closed--;
		} else if(prev == ']') {
			closed++;
		}
	}
	scanner.current++;
	// printf("Scanner jumped back to %d\n", scanner.current);
}
