#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<stdint.h>
#include<math.h>

int* stack = NULL;
int stack_size = 0;
int stack_pos = 0;

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
	if(new_size == 0){
		free(pointer);
		return NULL;
	}

	return realloc(pointer, new_size);
}

// Yes this leaks memory, the program is going to segfault anyway, so who really cares
void push(int val) {
	if(stack_pos == stack_size){
		stack = reallocate(stack, sizeof(int) * stack_size, sizeof(int) * (stack_size + 1) * 2);	
		stack_size = (stack_size + 1) * 2;
	}
	stack[++stack_pos] = val;
}

int pop() {
	int val = stack[stack_pos--];
	return val;
}

void print_stack() {
	printf("> Stack:\n");
	for(int i = 0; i<stack_pos; i++) {
		printf(">> [%d] = %d\n", i, stack[stack_pos]);
	}
}

void print_ret() {
	print_stack();
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}

// Pushes 1 onto the stack
void push_one() {
	push(1);
	print_ret();
}

// Pops two and adds them
void add() {
	int x = pop();
	int y = pop();
	push(x+y);
	print_ret();
}

// Just says hi
void print_hi() {
	printf("Hi\n");
	print_ret();
}

// Pops a value and says bye to it
void print_bye() {
	printf("Bye %d~\n", pop());
	print_ret();
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

	char* op_hi = generate_instr(print_hi);
	char* op_bye = generate_instr(print_bye);
	char* op_add = generate_instr(add);
	char* op_one = generate_instr(push_one);

	//debug_buf("print_hi_instr", print_hi_instr, 8);
	//debug_buf("print_bye_instr", print_bye_instr, 8);

	// Begin "compilation"	
	int start = prepare_exec(exec);

	start = write_instr(exec, op_hi, start);
	start = write_instr(exec, op_one, start);
	start = write_instr(exec, op_one, start);
	start = write_instr(exec, op_add, start);
	start = write_instr(exec, op_bye, start);

	//start = write_instr(exec, print_hi_instr, start);	
	//start = write_instr(exec, print_bye_instr, start);
}

int main() {
	char exec[0]; // Our rop target
	compile(exec);
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}
