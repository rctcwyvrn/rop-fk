#ifndef rop_scanner_h
#define rop_scanner_h

void init_scanner(char* source);

typedef enum {
	Token_R, Token_L, Token_Inc, Token_Dec, Token_Period, Token_Comma, Token_LB, Token_RB, Token_EOF
} TokenType;

TokenType scan_token();


void jump_forward();
void jump_back();

#endif
