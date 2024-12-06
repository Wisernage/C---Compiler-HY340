%{
#if defined(WIN32)
#define YY_NO_UNISTD_H
#elif defined(_WIN32_WCE)
#define YY_NO_UNISTD_H
#endif

#include <stdlib.h>
#include <iostream>
#include <string>
#include <stack>
#include "parser.hpp"


unsigned int tok_count = 1u;
unsigned int line_count = 1u;
std::stack<unsigned> cmnt_stack;

/*
	*@brief function for handling escape characters inside strings
*/
std::string* handle_string(){
	std::string *str = new std::string();
	str->resize(yyleng);
	int i = 0, j = 0;
	while(yytext[i] != 0) {
		if(yytext[i] == '\\') {// metatrepw 2 char se ena
			++i;
			str->resize(str->size()-1);
			switch(yytext[i]) {
				case 'n':
					str->at(j) = '\n';
					break;
				case 't':
					str->at(j) = '\t';
					break;
				case 'r':
					str->at(j) = '\r';
					break;
				case 'a':
					str->at(j) = '\a';
					break;
				case '\"':
					str->at(j) = '\"';
					break;
				case '\\':
					str->at(j) = '\\';
					break;
				case 'v':
					str->at(j) = '\v';
					break;
				case '0':
					str->at(j) = '\0';
					break;
				default :
					std::cout << "\033[33mWARNING\033[37m : invalid escape character detected "
					"in line: " << line_count << " string = " << yytext << std::endl;
					str->at(j) = yytext[i];
			}
		}
		else {
			str->at(j) = yytext[i];
		}
		++i;
		++j;
	}
	
	return str;
}

%}

%option noyywrap
%option yylineno


keyword			if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
integer			(0[xX][0-9A-Fa-f]+)|([0-9]+)
real			[0-9]+\.[0-9](e|E)?[0-9]*
string			\"([^\n\"]|"\\\"")*\"
identifier		[a-zA-Z][a-zA-Z_0-9]*
slcomment		"//".*
mlcomment		"/*"
space			[ \t]*

%x MLCOMMENT
%x C_STRING

%%

\n				{ line_count++; }
{space}			{;}

{slcomment}		{ ; }

{mlcomment}		{
					std::string str = std::to_string(line_count);
					str.append(" - ");
					cmnt_stack.push(tok_count++);
					BEGIN(MLCOMMENT);
				}

<MLCOMMENT>"/*"	{
					cmnt_stack.push(tok_count++);
				}

<MLCOMMENT>"*/" { 	
					if(!cmnt_stack.empty()){
						cmnt_stack.pop();
						if (cmnt_stack.empty()){
							BEGIN(INITIAL);
						}else{
							BEGIN(MLCOMMENT);
						}
					}else {
						BEGIN(INITIAL);
					}
				}

<MLCOMMENT>"\n"	{ line_count++; }

<MLCOMMENT>.  	{ ; }


"if"		{ return IF; }
"else"		{ return ELSE; }
"while"		{ return WHILE; }
"function"	{ return FUNCTION; }
"for"		{ return FOR; }
"return"	{ return RETURN; }
"break"		{ return BREAK; }
"continue"	{ return CONTINUE; }
"and"		{ return AND; }
"not"		{ return NOT; }
"or"		{ return OR; }
"local"		{ return LOCAL; }
"true"		{ return TRUE; }
"false"		{ return FALSE; }
"nil"		{ return NIL; }
"="			{ return ASSIGN; }
"+"			{ return PLUS; }
"-"			{ return MINUS; }
"*"			{ return MULT; }
"/"			{ return DIVIDE; }
"%"			{ return PERCENT; }
"++"		{ return PLUSPLUS; }
"--"		{ return MINUSMINUS; }
">"			{ return GREATER; }
"<"			{ return LESSER; }
">="		{ return GREATEREQUAL; }
"<="		{ return LESSEREQUAL; }
"=="		{ return EQUAL; }
"!="		{ return NOTEQUAL; }
"{"			{ return LCBRACK; }
"}"			{ return RCBRACK; }
"["			{ return LBRACK; }
"]"			{ return RBRACK; }
"("			{ return LPAREN; }
")"			{ return RPAREN; }
";"			{ return SEMICOLON; }
","			{ return COMMA; }
":"			{ return COLON; }
"::"		{ return COLONCOLON; }
"."			{ return DOT; }
".."		{ return DOTDOT; }

{integer}		{ 
					yylval.intConst = atoi(yytext);
					return INTEGER;
				}
{real}			{ 
					yylval.realConst = atof(yytext);
					return REAL;
				}
{string}		{ 
					yylval.strConst = handle_string();
					return STRING;
				}

{identifier}	{ 
					yylval.strConst = new std::string(yytext);
					return ID;
				}

.               { 
					std::string str(yytext);
					std::cout << "\033[31m" << "ERROR :" << "\033[37m";
					if(yytext[0] == '\"'){
						std::cout << " in line: " << line_count << ", string literal is not closing\n";
					}
					else{
						std::cout << " in line: " << line_count << ", Undefined character token : " << yytext[0] << std::endl;
					}
				}

<<EOF>>			{ return 0; }

%%

void validate_comments() {
	int indx = 0;
	while (!cmnt_stack.empty()) {
		indx = cmnt_stack.top();
		cmnt_stack.pop();
	}
	if (indx){
		std::cout << "\033[31m" << "ERROR :" << "\033[37m" << 
		" comment is not closing\n";
	}
}
