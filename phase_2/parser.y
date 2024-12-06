%{

/* 
						 CS340 - Compilers ~ Project Spring 2022
					  Computer Of Crete, Computer Science Department

							Constantine Damaskinakis - csd3755
							   Minos Stavrakakis - csd4120
							 Demetrious Grammenidis - csd3933

 */

#include <assert.h>
#include <iostream>
#include <string>
#include "symtable.h"
#include <unordered_map>

bool member_flag = false;
int yyerror(std::string message);

extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int yylex();

/* Auxiliary var for storing each rule's id value (e.g. entry["rule8.1"] = id) returned
 * from lookup (e.g at $1) */
std::unordered_map<std::string, struct st_entry*> st_entry_tmp;
/* In alpha a funcdef can be inside a function's scope so we needed to know if we are
 * inside a function 
 */
extern std::stack<struct st_entry*> func_stack;

void print_rules(std::string str) {
	 /* std::cout << "~ entered rule :\t " << str << std::endl; */
}

%}

%defines
%output "parser.cpp"

%union{
	int intConst;
	double realConst;
	std::string *strConst;
	struct st_entry *st_entryVal;
	bool boolean;
}

%token<intConst> INTEGER 
%token<realConst> REAL
%token<strConst> STRING
%token<strConst> ID
%token<boolean> TRUE FALSE	/* This may be unnecessary */
%token OR LOCAL NIL UMINUS MINUSMINUS
%token IF ELSE WHILE FUNCTION FOR RETURN BREAK CONTINUE AND NOT 
%token ASSIGN PLUS MINUS MULT DIVIDE PERCENT NOTEQUAL PLUSPLUS
%token GREATER  LESSER GREATEREQUAL LESSEREQUAL EQUAL
%token LCBRACK RCBRACK LBRACK RBRACK LPAREN RPAREN SEMICOLON COMMA COLON COLONCOLON DOT DOTDOT

%type program stmts stmt expr term assignexpr primary
%type member call elist objectdef const returnstmt
%type idlist

%type<st_entryVal> funcdef 
%type<st_entryVal> lvalue


%right 		ASSIGN
%left 		OR
%left 		AND
%nonassoc 	EQUAL NOTEQUAL
%nonassoc 	GREATER  LESSER GREATEREQUAL LESSEREQUAL
%left 		PLUS MINUS
%left 		MULT DIVIDE PERCENT
%right 		NOT PLUSPLUS MINUSMINUS UMINUS
%left 		DOT DOTDOT
%left 		LBRACK RBRACK
%left 		LPAREN RPAREN

%start program
%expect 1

%%

program:	  stmts						{std::cout << "Finished reading statements\n";}
			;

// Rule 2.
stmts		: stmts stmt 				{print_rules("2.1 stmts -> stmts stmt");}
			|							{print_rules("2.2 stmts -> ε");}
			;
// Rule 3.
stmt		: expr SEMICOLON			{print_rules("3.1 stmt -> expr ;");}
			| ifstmt					{print_rules("3.2 stmt -> ifstmt");}
			| whilestmt					{print_rules("3.3 stmt -> whilestmt");}
			| forstmt					{print_rules("3.4 stmt -> forstmt");}
			| returnstmt				{print_rules("3.5 stmt -> returnstmt");}
			| BREAK SEMICOLON			{print_rules("3.6 stmt -> BREAK ;");}
			| CONTINUE SEMICOLON		{print_rules("3.7 stmt -> CONTINUE ;");}
			| block						{print_rules("3.8 stmt -> block");}
			| funcdef					{print_rules("3.9 stmt -> funcdef");}
			| SEMICOLON					{print_rules("3.10 stmt -> ;");}
			;

// Rule 4.
expr		: assignexpr				{print_rules("4.1 expr -> assignexpr");}
			| expr PLUS expr			{print_rules("4.2 expr -> expr + expr");}
			| expr MINUS expr			{print_rules("4.3 expr -> expr - expr");}
			| expr MULT expr			{print_rules("4.4 expr -> expr * expr");}
			| expr DIVIDE expr			{print_rules("4.5 expr -> expr / expr");}
			| expr PERCENT expr			{print_rules("4.6 expr -> expr \% expr");}
			| expr GREATER expr			{print_rules("4.7 expr -> expr > expr");}
			| expr GREATEREQUAL expr	{print_rules("4.8 expr -> expr >= expr");}	
			| expr LESSER expr			{print_rules("4.9 expr -> expr < expr");}
			| expr LESSEREQUAL expr		{print_rules("4.10 expr -> expr <= expr");}	
			| expr EQUAL expr			{print_rules("4.11 expr -> expr == expr");}	
			| expr NOTEQUAL expr		{print_rules("4.12 expr -> expr != expr");}
			| expr AND expr				{print_rules("4.13 expr -> expr AND expr");}
			| expr OR expr				{print_rules("4.14 expr -> expr OR expr");}
			| term						{print_rules("4.15 expr -> term");}
			;
// Rule 5.
term		: LPAREN expr RPAREN		{print_rules("5.1 term -> ( expr )");}
			| MINUS expr %prec UMINUS	{print_rules("5.2 term -> - expr");}
			| NOT expr					{print_rules("5.3 term -> NOT expr");}
			| PLUSPLUS lvalue			{
											print_rules("5.4 term -> ++ lvalue");
											if($2->type == USER_FUNC || $2->type == LIB_FUNC){
												yyerror("functions are constant, their value cannot be changed");
											}
										}
			| lvalue PLUSPLUS			{
											print_rules("5.5 term -> lvalue ++");
											if($1->type == USER_FUNC || $1->type == LIB_FUNC){
												yyerror("functions are constant, their value cannot be changed");
											}
										}
			| MINUSMINUS lvalue			{
											print_rules("5.6 term -> -- lvalue");
											if($2->type == USER_FUNC || $2->type == LIB_FUNC){
												yyerror("functions are constant, their value cannot be changed");
											}
										}
			| lvalue MINUSMINUS			{
											print_rules("5.7 term ->  lvalue --");
											if($1->type == USER_FUNC || $1->type == LIB_FUNC){
												yyerror("functions are constant, their value cannot be changed");
											}
										}
			| primary					{print_rules("5.8 term -> primary");}
			;
// Rule 6.
assignexpr	: lvalue ASSIGN expr		{
											print_rules("6.1 assignexpr -> lvalue = expr");
		   							 		if(!member_flag && $1 && ($1->type==LIB_FUNC || $1->type==USER_FUNC) ) 
												 yyerror("invalid assignment (lvalue is a function)");
											if(member_flag)
												member_flag = false;
										}
			;

// Rule 7.
primary		: lvalue					{print_rules("7.1 primary -> lvalue");}
			| call						{print_rules("7.2 primary -> call");}
			| objectdef					{print_rules("7.3 primary -> objectdef");}
			| LPAREN funcdef RPAREN		{print_rules("7.4 primary -> ( funcdef )");}
			| const						{print_rules("7.5 primary -> const");}
			;
// Rule 8.
lvalue		: ID						{
											print_rules("8.1 lvalue -> ID");
											st_entry_tmp["r8"] = st_lookup(*$1);
											if(!st_entry_tmp["r8"]){
												$$ = st_insert(*$1, (st_get_scope() == 0) ? GLOBAL_VAR : LOCAL_VAR);
											}
											else if( (st_entry_tmp["r8"]->scope != 0) && st_entry_tmp["r8"]->type != USER_FUNC
													&& !func_stack.empty() && 
													(st_entry_tmp["r8"]->scope <= func_stack.top()->scope) ){
												yyerror("Cannot access local var \'"+*$1+"\' inside function \'"
												+func_stack.top()->name + "\'");
												$$ = NULL;
											}
											else{
												$$ = st_entry_tmp["r8"];
											}
										}
			| LOCAL ID					{
											print_rules("8.2 lvalue -> local ID");
											st_entry_tmp["r8"] = st_lookup(*$2);
											if(st_entry_tmp["r8"] && (st_entry_tmp["r8"]->scope == st_get_scope()) ){
												$$ = st_entry_tmp["r8"];
											}
											else if(st_entry_tmp["r8"] && st_entry_tmp["r8"]->type == LIB_FUNC){
												yyerror("variable \'" + *$2 + "\' shadows lib function");
											}
											else{
												$$ = st_insert(*$2, (st_get_scope() == 0) ? GLOBAL_VAR : LOCAL_VAR);
											}
										}
			| COLONCOLON ID				{
											print_rules("8.3 lvalue -> ::ID");
											st_entry_tmp["r8"] = st_lookup(*$2, 0);
											if(!st_entry_tmp["r8"]){
												yyerror("No global variable "+*$2+" exists.");
												$$ = NULL;
											}
											else {
												$$ = st_entry_tmp["r8"];
											}
										}
			| member					{
											print_rules("8.4 lvalue -> member");

										}
			;
// Rule 9.
member		: lvalue DOT ID				{
											print_rules("9.1 member -> lvalue . ID");
											member_flag = true;
										}
			| lvalue LBRACK expr RBRACK	{
											print_rules("9.2 member -> lvalue [ expr ]");
											member_flag = true;
										}
			| call DOT ID				{
											print_rules("9.3 member -> call . ID");
											member_flag = true;
										}
			| call LBRACK expr RBRACK 	{
											print_rules("9.4 member -> call [ expr ]");
											member_flag = true;
										}
			;
// Rule 10.
call		: call LPAREN elist RPAREN	{print_rules("10.1 member -> call ( elist )");}
			| lvalue callsuffix			{print_rules("10.2 member -> lvalue callsuffix");}
			| LPAREN funcdef RPAREN LPAREN elist RPAREN
										{print_rules("10.3 member -> ( funcdef ) ( elist )");}
			;
// Rule 11.
callsuffix	: normcall					{print_rules("11.1 member -> normcall");}
			| methodcall				{print_rules("11.2 member -> methodcall");}
			;
// Rule 12.
normcall	: LPAREN elist RPAREN		{print_rules("12.1 normcall -> ( elist )");}
			;
// Rule 13.
methodcall	: DOTDOT ID LPAREN elist RPAREN{print_rules("13.1 methodcall -> .. ID ( elist )");}
			;
// Rule 14.
elist		: expr 						{print_rules("14.1 elist -> expr");}
			| elist COMMA expr 			{print_rules("14.2 elist -> elist , expr");}
			| 							{print_rules("14.3 elist -> ε");}
			;
// Rule 15.
objectdef	: LBRACK elist RBRACK 		{print_rules("15.1 objectdef -> [ elist ]");}
			| LBRACK indexed RBRACK 	{print_rules("15.2 objectdef -> [ indexed ]");}
			;
// Rule 16.
indexed		: indexedelem 				{print_rules("16.1 indexed -> indexedelem");}
			| indexed COMMA indexedelem	{print_rules("16.2 indexed ->  indexed , indexedelem");}
			;
// Rule 17.
indexedelem	: LCBRACK expr COLON expr 	
			  RCBRACK					{print_rules("17.1 indexedelem -> { expr : expr }");}
			;
// Rule 18.
block		: LCBRACK 					{ 	print_rules("18.1 block -> { stmts }");
											st_increase_scope();}
	   		  stmts RCBRACK 			{
											st_hide(st_get_scope()); st_decrease_scope();
										}
			;
// Rule 19.
funcdef		: FUNCTION 					{	print_rules("19.1 funcdef -> function ( idlist ) block");
											st_entry_tmp["r19"] = st_insert(st_godfather(), USER_FUNC);
											func_stack.push(st_entry_tmp["r19"]);
										}
			  LPAREN					{st_increase_scope();}
			  idlist RPAREN				{
				  							
				  							offload_arglist(st_entry_tmp["r19"]);
											st_decrease_scope();
										}
			  block 					{
											func_stack.pop();
										}
			| FUNCTION 					{	
											print_rules("19.2 funcdef -> function ID ( idlist ) block");
											
										}
			  ID 						{	
											st_entry_tmp["r19"] = st_lookup(*$3);
											if((!st_entry_tmp["r19"]) ||
												( (st_entry_tmp["r19"]->scope < st_get_scope())
												&& (st_entry_tmp["r19"]->type != LIB_FUNC) ) )
											{
												st_entry_tmp["r19"] = st_insert(*$3, USER_FUNC);
												func_stack.push(st_entry_tmp["r19"]);// push to func stack mono an einai valid
											}
											else
											{
												if(st_entry_tmp["r19"]->type == USER_FUNC)// kseroume oti einai locally defined
													yyerror("redefinition of user function defined in line "
													+ std::to_string(st_entry_tmp["r19"]->line));
												else if(st_entry_tmp["r19"]->type == LIB_FUNC){
													yyerror("function definition shadows lib function");
												}
												else if(st_entry_tmp["r19"]->type == LOCAL_VAR
														|| st_entry_tmp["r19"]->type == FORMAL_ARG
														|| st_entry_tmp["r19"]->type == GLOBAL_VAR){
													yyerror("variable \""+ *$3 + "\" already defined in line "
													+std::to_string(st_entry_tmp["r19"]->line));
												}
												else{	/* Exei vre8ei to active token, den einai user i lib func,
														 den einai active variable  */
													yyerror("UNHANDLED CASE ?\nonoma: " + st_entry_tmp["r19"]->name +
													" typos: " + std::to_string(st_entry_tmp["r19"]->type) + 
													" grammh: " + std::to_string(st_entry_tmp["r19"]->line));
												}
												st_entry_tmp["r19"] = NULL;
											}
										}
			  LPAREN					{	st_increase_scope();		} 
			  idlist RPAREN				{
				  							if(st_entry_tmp["r19"]){
												offload_arglist(st_entry_tmp["r19"]);
											}
											st_decrease_scope();
										}
			 block 						{
											if(st_entry_tmp["r19"])
												func_stack.pop();
										}
			 ;
// Rule 20.
const		: INTEGER 					{print_rules("20.1 const -> INTEGER");}	
	   		| REAL 						{print_rules("20.2 const -> REAL");}
			| STRING 					{print_rules("20.3 const -> STRING");}
			| NIL 						{print_rules("20.4 const -> NIL");}
			| TRUE 						{print_rules("20.5 const -> TRUE");}
			| FALSE						{print_rules("20.6 const -> FALSE");}
			;
// Rule 21.
idlist		: ID 						{
											print_rules("22.1 idlist -> ID");
											st_entry_tmp["r21"] = st_lookup(*$1, 0);
											if(st_entry_tmp["r21"] && st_entry_tmp["r21"]->type ==  LIB_FUNC){
												yyerror("formal argument " + *$1 + " shadows lib func");
											}
											else{
												st_entry_tmp["r21"] = st_insert(*$1, FORMAL_ARG);
												load_2_arglist(st_entry_tmp["r21"]);
											}
										}
			| idlist COMMA ID 			{
											print_rules("22.2 idlist -> idlist , ID");
											st_entry_tmp["r21"] = st_lookup(*$3);
											if(st_entry_tmp["r21"] && (st_entry_tmp["r21"]->type == FORMAL_ARG)){ // conflict
												yyerror("Argument "+ *$3 +" already exists.");
											}
											else if(st_entry_tmp["r21"] && st_entry_tmp["r21"]->type == LIB_FUNC){
												yyerror("formal argument " + *$3 + " shadows lib func");
											}
											else{
												st_entry_tmp["r21"] = st_insert(*$3, FORMAL_ARG);
												load_2_arglist(st_entry_tmp["r21"]);
											}										
										}
			|
			;
// Rule 23.
ifstmt		: IF LPAREN expr RPAREN stmt{print_rules("23.1 ifstmt -> if ( expr ) stmt");}
			| IF LPAREN expr RPAREN stmt ELSE stmt{print_rules("23.2 ifstmt -> if ( expr ) stmt else stmt");}
			;
// Rule 24.
whilestmt	: WHILE LPAREN expr RPAREN stmt{print_rules("24.1 whilestmt -> while ( expr ) stmt");}
			;
// Rule 25.
forstmt		: FOR LPAREN elist SEMICOLON expr SEMICOLON elist RPAREN stmt {print_rules("25.1 forstmt -> for ( elist ; expr ; elist ) stmt");}
			;
// Rule 26.
returnstmt 	: RETURN SEMICOLON 			{
											print_rules("26.1 returnstmt -> return ;");
											if (func_stack.empty()){
												yyerror("Use of 'return' while not in a function\n");
											}
										}
			| RETURN expr SEMICOLON 	{
											print_rules("26.2 returnstmt -> return expr ;");
											if (func_stack.empty()){
												yyerror("Use of 'return' while not in a function\n");
											}
										}
			;

%%

extern void validate_comments();
			
int yyerror(std:: string err){
	std::cout << "\033[31m" << "ERROR " << "\033[37m" <<
	"in line " << yylineno << " : " << err << "\n";
	return 1;
}

int main(int argc, char** argv) {
	std::cout << "\033[37m";
    if (argc > 1) {
		if (!(yyin = fopen(argv[1], "r"))) {
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}
	} else {
		yyin = stdin;
	}
	st_initialize();
    while( yyparse() != 0);
	validate_comments();
	st_print_table();

    return 0;
}
