#pragma once
#include <stdlib.h>
#include <iostream>
#include <string>

enum token_t_enum {
	UNDEFINED       =0,
    KEYWORD         =1,
	IDENTIFIER      =2,
	OPERATOR        =3,
	INTCONST        =4,
	DOUBLECONST     =5,
	STRINGCONST     =6,
	PUNCTUATION     =7,
	LINE_COMMENT    =8,
	BLOCK_COMMENT   =9,
	NESTED_COMMENT  =10
};

enum tok_error_t {
	FLAWLESS	=0,
	ERROR 		=1,
	WARNING 	=2
};

typedef struct alpha_token_t {
	unsigned tok_line;
	unsigned long tok_num;
	std::string tok_val;
	enum tok_error_t tok_err_t;
	std::string tok_debug; /* added to print various useful messages */
	enum token_t_enum tok_type;
} alpha_token_t;
