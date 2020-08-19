#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "scanner.h"

#define DEBUG false

int* stack;
int stack_size = 1;
int stack_pos = 0;

char* exec;
int exec_pos = 0;

char* op_r; 
char* op_l;
char* op_inc;
char* op_dec;
char* op_out;
char* op_in;
char* op_lb;
char* op_rb;

char* op_fin;
char* op_next;

void debug_buf(char* name, char* exec, int len) {
	printf("%s: [", name);
	for(int i=0;i<len; i++){
		printf("%hhx", exec[i]);
	}
	printf("] | cur_len = %d\n", len);

}

void debug() {
	debug_buf("exec", exec, exec_pos);
}


void print_stack() {
	printf("\n> Stack{%d}:\n", stack_pos);
	for(int i = 0; i<stack_size; i++) {
		printf(">> [%d] = %d\n", i, stack[i]);
	}
}

void print_ret() {
	print_stack();
	printf(">> Returning to %p\n",  __builtin_return_address(1));
}

void write_instr(char* instr) {
	for(int i=0;i<8;i++) {
		//printf("Writing %hhx to pos %d\n", instr[i], i+exec_pos);
		exec[i + exec_pos] = instr[i];
	}
	exec_pos +=8;
	if (DEBUG) { 
		// debug_buf("next instr", instr, 8); 
		// debug();
	}
}

void next() {
	TokenType next_op = scan_token();
	switch(next_op) {
		case Token_R: 
			write_instr(op_r); break;
		case Token_L: 
			write_instr(op_l); break;
		case Token_Inc:;	
			write_instr(op_inc); break;
		case Token_Dec: 
			write_instr(op_dec); break;
		case Token_Period: 
			write_instr(op_out); break;
		case Token_Comma: 
			write_instr(op_in); break;
		case Token_LB: 
			write_instr(op_lb); break;
		case Token_RB: 
			write_instr(op_rb); break;
		case Token_EOF: 
			write_instr(op_fin); break;
	}

	write_instr(op_next);

	if(DEBUG) { 
		printf("Next: %d", next_op);
		print_ret();	
	}

}

// Instructions
// =====
void move_right() {
	if(stack_pos + 1 == stack_size){
		stack = realloc(stack, sizeof(int) * stack_size * 2);
		stack[stack_pos + 1] = 0;	
		stack_size *= 2;
	}

	stack_pos++;
}

void move_left(){
	stack_pos--;
}

void increment(){
	stack[stack_pos]++;
}

void decrement(){
	stack[stack_pos]--;
}

void output(){
	int val = stack[stack_pos];
	printf("%c", (char) val);
}

void input(){
	int c = getchar();
	stack[stack_pos] = c;
}

void left_square(){
	int val = stack[stack_pos];
	if(val == 0) {
		jump_forward();
	}	
}

void right_square(){
	int val = stack[stack_pos];
	if(val != 0) {
		jump_back();
	}
}

// Cleanly exit
void cleanup(){
	printf("\n> Execution done \n");
	exit(0);
}

char* generate_instr(void* fn_ptr){
	unsigned int num = (uintptr_t) fn_ptr;
	//int num = (int) fn_ptr;
	int n = (int) floor(log(num)/log(256)) + 1;
	char* instr = malloc(8*sizeof(char));

	for(int i = 0; i < n; i++, num /= 256 ) {
		instr[i] = num % 256;
		//printf("Just put %hhx onto the instr\n", num % 256);
	}
	for(int i = n; i < 8; i++) {
		instr[i] = 0; // Zero out the rest of the instr
	}
	//debug_buf("generated", instr, 8);	
	return instr;
}

void prepare_exec() {
	int initial_smash = 8;
	for(int i=0; i<initial_smash; i++) {
		exec[i] = 0x41; // Overflow some stack 
	}
	// debug(exec, initial_run);
	exec_pos = initial_smash;
}

void interpret(char* source){
	// Initialize the stack
	stack = malloc(sizeof(int));
	stack[0] = 0;

	// Initialize the opcodes
	op_r = generate_instr(move_right);
	op_l = generate_instr(move_left);
	op_inc = generate_instr(increment);
	op_dec = generate_instr(decrement);
	op_out = generate_instr(output);
	op_in = generate_instr(input);
	op_rb = generate_instr(right_square);
	op_lb = generate_instr(left_square);

	op_fin = generate_instr(cleanup);
	
	op_next = generate_instr(next);

	// debug help
	if(DEBUG) {
		printf("Location of >: %p\n",move_right);
		printf("Location of <: %p\n",move_left);
		printf("Location of +: %p\n",increment);
		printf("Location of -: %p\n",decrement);
		printf("Location of .: %p\n",output);
		printf("Location of ,: %p\n",input);
		printf("Location of [: %p\n",left_square);
		printf("Location of ]: %p\n",right_square);
	}

	// Initialize the scanner
	init_scanner(source);

	prepare_exec();	
	write_instr(op_next);
}

static char* read_file(const char* path) {
	FILE* file = fopen(path, "rb");
	
	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	
	char* buffer = (char*)malloc(fileSize + 1);
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

static void run_file(char* path) {
	char* source = read_file(path);
	printf("Source: %s", source);
	interpret(source);
}

// Creates a bunch of empty stack frames that we can destroy
// Any more and we get a stack overflow
void create_exec(int depth, char* path) {
	if(depth == 0){
		// Prepare exec	
		char target[0]; // Our rop target
		exec = target;
	
		run_file(path);
		if (DEBUG) {printf(">> Returning to %p\n",  __builtin_return_address(0));}
		sleep(0.5);
	} else {
		char space[800];
		space[799] = 'c'; // Be absolutely sure that gcc won't optimize this out of existence
		create_exec(depth - 1, path);
	}
}

int main(int argc, char* argv[]) {
	if(argc == 2) {
		create_exec(10000, argv[1]);
	} else {
		fprintf(stderr, "Usage: rop [path] \n");
		exit(64);
	}
}
