%{
#if defined(WIN32)
#define YY_NO_UNISTD_H
#elif defined(_WIN32_WCE)
#define YY_NO_UNISTD_H
#endif

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <climits> // this is added to allow uns lng cast to int
#include <cassert>
#include "alpha_token_t.h"

#define YY_DECL int alpha_yylex (void* yylval)


unsigned long tok_count = 1lu;
unsigned int line_count = 1u;
std::stack<unsigned long> cmnt_stack;
std::vector<alpha_token_t> tok_vec;

/*
	*@brief function for handling escape characters inside strings
*/
void handle_string(){
	std::string str, debug;
	debug = std::string("Invalid escape characters detected: ");
	str.resize(yyleng);
	int i = 0, j = 0;
	enum tok_error_t flag = FLAWLESS;
	while(yytext[i] != 0) {
		if(yytext[i] == '\\') {
			++i;
			switch(yytext[i]){
				case 'n':
					str.at(j) = '\n';
					break;
				case 't':
					str.at(j) = '\t';
					break;
				case 'a':
					str.at(j) = '\a';
					break;
				case '\"':
					str.at(j) = '\"';
					break;
				case '\\':
					str.at(j) = '\\';
					break;
				case 'v':
					str.at(j) = '\v';
					break;
				case '0':
					str.at(j) = '\0';
					break;
				default :	// for printing the bad esc chars
					debug.append((yytext+i-1),2);
					debug.append("  ");
					flag = WARNING;
					str.at(j) = yytext[i];
			}
		}
		else {
			str.at(j) = yytext[i];
		}
		++i;
		++j;
	}
	alpha_token_t tok{
		line_count,
		tok_count++,
		str.substr(0,j+1),
		flag,
		debug,
		STRINGCONST,
	};
	tok_vec.push_back(tok);
	/* if(yytext[i-2] == '\\'){ */
	/* 	debug = "string literal is not closing"; */
	/* 	alpha_token_t tok{ */
	/* 		line_count, */
	/* 		tok_count++, */
	/* 		str.substr(0, j+1), */
	/* 		ERROR, */
	/* 		debug, */
	/* 		STRINGCONST, */
	/* 	}; */
	/* 	tok_vec.push_back(tok); */
	/* } */
}

%}

%option noyywrap
%option yylineno


keyword			if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
operator		[=\+\-\*\/\%\>\<]|"=="|"!="|"++"|"--"|">="|"<="
integer			(0[xX][0-9A-Fa-f]+)|([0-9]+)
real			[0-9]+\.[0-9](e|E)?[0-9]*
string			\"([^\n\"]|"\\\"")*\"
punctuation		"::"|".."|[\{\}\[\]\(\)\;\,\:\.]
identifier		[a-zA-Z][a-zA-Z_0-9]*
slcomment		"//".*
mlcomment		"/*"
space			[ \t]*

%x MLCOMMENT

%%

\n				{ line_count++; }
{space}			{;}
{keyword}		{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						KEYWORD
					};
					tok_vec.push_back(tok);
					return 1;
				}
{operator}		{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						OPERATOR
					};
					tok_vec.push_back(tok);
					return 1;
				}
{integer}		{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						INTCONST,
					};
					tok_vec.push_back(tok);
					return 1;
				}
{real}			{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						DOUBLECONST
					};
					tok_vec.push_back(tok);
					return 1;
				}
{string}		{ 
					handle_string();
					return 1;
				}

{punctuation}	{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						PUNCTUATION
					};
					tok_vec.push_back(tok);
					return 1;
				}
{identifier}	{ 
					std::string str(yytext);
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						FLAWLESS,
						"",
						IDENTIFIER
					};
					tok_vec.push_back(tok);
					return 1;
				}
{slcomment}		{ 
					alpha_token_t tok{
						line_count,
						tok_count++,
						"//",
						FLAWLESS,
						"",
						LINE_COMMENT
					};
					tok_vec.push_back(tok);
					return 1;
				}
.               { 
					std::string str(yytext), debug;
					enum token_t_enum category = UNDEFINED;
					if(yytext[0] == '\"'){
						debug = "string literal is not closing";
						category = STRINGCONST;
					}
					else{
						debug = std::string("Undefined character token : ") += yytext[0];
					}
					alpha_token_t tok{
						line_count,
						tok_count++,
						str,
						ERROR,
						debug,
						category
					};
					tok_vec.push_back(tok);
					return 1;
				}

{mlcomment}			{
						std::string str = std::to_string(line_count);
						str.append(" - ");
						alpha_token_t tok{
							line_count,
							tok_count++,
							str, // writting the line where cmnt begins
							FLAWLESS,
							"",	// tok_debug
							(cmnt_stack.empty()) ? BLOCK_COMMENT : NESTED_COMMENT
						};
						tok_vec.push_back(tok);
						cmnt_stack.push(tok_count);
						BEGIN(MLCOMMENT);
					}

<MLCOMMENT>"\n"		{ line_count++; }

<MLCOMMENT>"/"+"*"	{
						std::string str = std::to_string(line_count);
						str.append(" - ");
						alpha_token_t tok{
							line_count,
							tok_count++,
							str, // writting the line where cmnt begins
							FLAWLESS,
							"",	// tok_debug
							(cmnt_stack.empty()) ? BLOCK_COMMENT : NESTED_COMMENT
						};
						tok_vec.push_back(tok);
						cmnt_stack.push(tok_count);
					}

<MLCOMMENT>"*"+"/"	{ 	
						if (!cmnt_stack.empty()) {
							int indx = cmnt_stack.top() & INT_MAX;
							tok_vec[indx-2].tok_val.append(std::to_string(line_count));
							cmnt_stack.pop();
							if (cmnt_stack.empty()){
								BEGIN(INITIAL);
							}else{
								BEGIN(MLCOMMENT);
							}
						}
						else {
							alpha_token_t tok{
								line_count,
								tok_count++,
								"*/",
								ERROR,
								"Found end of block comment without having an open comment block.",	// tok_debug
								UNDEFINED
							};
							tok_vec.push_back(tok);
						}
				 	}
<MLCOMMENT>.  	{ ; }

<<EOF>>			{ return 0; }

%%

void print_tok_vector(const std::vector<alpha_token_t>& tok_vec){	
	std::cout << "\033[37m";
	const char* tok_types[] = { "UNDEFINED", "KEYWORD", "IDENTIFIER", "OPERATOR",
						"INTCONST", "DOUBLECONST", "STRINGCONST", "PUNCTUATION",
						"LINE_COMMENT", "BLOCK_COMMENT", "NESTED_COMMENT" };

	const char* tok_error_t[] = { "FLAWLESS", "ERROR", "WARNING" };

	std::cout << "------------------------- Lexical Analysis -----------------------------------------\n";
	
	for (auto t : tok_vec){
	 	
		std::cout << t.tok_line << ":\t#" << t.tok_num 
			<< "\t" << t.tok_val << "\t" << tok_types[t.tok_type] << "\t";
		if(t.tok_err_t == WARNING) {
			std::cout << "\033[33m" << tok_error_t[t.tok_err_t] << ": ";
			std::cout << "\033[37m";
			std::cout << t.tok_debug;
		}
		else if(t.tok_err_t == ERROR) {
			std::cout << "\033[31m" << tok_error_t[t.tok_err_t] << ": ";
			std::cout << "\033[37m";
			std::cout << t.tok_debug;
		}
		std::cout << std::endl;
	}
}

int validate_comments() {
	
	int indx = 0;
	while (!cmnt_stack.empty()) {
		indx = cmnt_stack.top() & INT_MAX;
		tok_vec[indx-2].tok_err_t = ERROR;
		tok_vec[indx-2].tok_debug = "Non-closing comment block.";
		cmnt_stack.pop();
	}
	return ((indx) ? -1 : 0);
}


int main(int argc, char** argv) {

    if (argc > 1) {
		if (!(yyin = fopen(argv[1], "r"))) {
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}
	} else {
		yyin = stdin;
	}

    while(alpha_yylex(NULL) != 0);
	validate_comments();
	print_tok_vector(tok_vec);

    return 0;
}
