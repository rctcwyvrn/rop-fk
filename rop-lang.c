#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<stdint.h>
#include<math.h>

void print_hi() {
	printf("Hi\n");
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}

void print_bye() {
	printf("Bye~\n");
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}

char* generate_instr(void* fn_ptr){
	unsigned int num = (uintptr_t) fn_ptr;
	int n = (int) floor(log(num)/log(256)) + 1;
	char* instr = malloc(n*sizeof(char));

	for(int i = 0; i < n; i++, num /= 256 ) {
		instr[i] = num % 256;
	}
	
	return instr;
}

void debug_buf(char* name, char* exec, int len) {
	printf("%s: [", name);
	for(int i=0;i<len; i++){
		printf("%hhx", exec[i]);
	}
	printf("] | cur_len = %d\n", len);

}

void debug(char* exec, int len) {
	debug_buf("exec", exec, len);
}

int prepare_exec(char* exec) {
	int initial_run = 8;
	for(int i=0; i<initial_run; i++) {
		exec[i] = 0x41; // Overflow some stack 
	}
	debug(exec, initial_run);
	return initial_run;
}

int write_instr(char* exec, char* instr, int start_pos) {
	for(int i=0;i<8;i++) {
		printf("Writing %hhx to pos %d\n", instr[i], i+start_pos);
		exec[i + start_pos] = instr[i];
	}
	debug(exec, start_pos + 8);
	return start_pos + 8;
}

void compile(char* exec) {
	printf("Location of print_hi %p\n", print_hi);
	printf("Location of print_bye %p\n", print_bye);

	char* print_hi_instr = generate_instr(print_hi);
	char* print_bye_instr = generate_instr(print_bye);

	debug_buf("print_hi_instr", print_hi_instr, 8);
	debug_buf("print_bye_instr", print_bye_instr, 8);

	// Begin "compilation"	
	int start = prepare_exec(exec);
	start = write_instr(exec, print_hi_instr, start);	
	start = write_instr(exec, print_bye_instr, start);
}

int main() {
	char exec[0]; // Our rop target
	compile(exec);
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}
