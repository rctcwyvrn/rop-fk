#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<stdint.h>
#include<math.h>
#include<unistd.h>

#define DEBUG (1!=1)

int* stack = NULL;
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
	printf("\n> Stack{%d}:\n", stack_size);
	for(int i = 0; i<stack_size; i++) {
		printf(">> [%d] = %d\n", i, stack[i]);
	}
}

void print_ret() {
	print_stack();
	printf(">> Returning to %p\n",  __builtin_return_address(0));
}


// Instructions
// =====

void move_right() {
	if(stack_pos + 1 == stack_size){
		stack = reallocate(stack, sizeof(int) * stack_size, sizeof(int) * (stack_size + 1) * 2);
		stack[stack_pos + 1] = 0;	
		stack_size = (stack_size + 1) * 2;
	}

	stack_pos++;
	if(DEBUG){ print_ret(); }
}

void move_left(){
	stack_pos--;
	if(DEBUG){ print_ret(); }
}

void increment(){
	stack[stack_pos]++;
	if(DEBUG){ print_ret(); }
}

void decrement(){
	stack[stack_pos]--;
	if(DEBUG){ print_ret(); }
}

void output(){
	int val = stack[stack_pos];
	printf("%d",val);
	
	if(DEBUG){ print_ret(); }
}

void input(){
	int c = getchar();
	stack[stack_pos] = c;	
	if(DEBUG){ print_ret(); }
}

// uhhhh i have no idea how control flow is going to work
// I feel like jumping forward may be possible with an gadget that just pops return addresses off the stack
// Jumping backwards seems to be impossible because once the return values are popped off, they're gone into the ether
//
// Maybe I could get it to work by collecting the instructions into a chunk, and then performing a rop for each new instruction
// So each instruction would be
// 1. Do something
// 2. Get the next instruction from a chunk
// 3. Overflow it onto exec



// Helps make sure all prints are completed before the segfault happens
void cleanup(){
	printf("\n> Execution done \n");
	sleep(1);
	// print_ret();
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

void debug() {
	debug_buf("exec", exec, exec_pos);
}

void prepare_exec() {
	int initial_smash = 8;
	for(int i=0; i<initial_smash; i++) {
		exec[i] = 0x41; // Overflow some stack 
	}
	// debug(exec, initial_run);
	exec_pos = initial_smash;
}

void write_instr(char* instr) {
	for(int i=0;i<8;i++) {
		// printf("Writing %hhx to pos %d\n", instr[i], i+start_pos);
		exec[i + exec_pos] = instr[i];
	}
	// debug(exec, start_pos + 8);
	exec_pos +=8;
}

void compile(){
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

	char* op_cleanup = generate_instr(cleanup);
	char op_fin[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// Begin "compilation"	
	prepare_exec();
	write_instr(op_in);
	write_instr(op_inc);
	write_instr(op_out);

	write_instr(op_cleanup);
	write_instr(op_fin);
}

int main() {
	char target[0]; // Our rop target
	exec = target;
	compile();
	if(DEBUG){ print_ret(); }
}
