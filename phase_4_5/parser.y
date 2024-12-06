%{

/* 
						  Computer Science Department
					CS340 - Compilers ~ Project Spring 2022
					   Constantine Damaskinakis - csd3755
						  Minoas Stavrakakis - csd4120
						Demetrious Grammenidis - csd3933

 */
#include <assert.h>
#include <iostream>
#include <string>
#include <cstring>
#include "symtable.h"
#include "quads.h"
#include "actions.h"
#include "scoping.h"
#include "avm_structures.h"
#include <unistd.h>
#include <unordered_map>


int yyerror(std::string message);
bool hasError = false;

extern std::vector<quad> quad_vec;
/* extern std::stack<int> loop_stack; */
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int yylex();
extern unsigned tmp_var_count;
extern void generate();

union values emptyval;

/* Auxiliary var for storing each rule's id value (e.g. entry["rule8.1"] = id) returned
 * from lookup (e.g at $1) */
std::unordered_map<std::string, struct st_entry*> st_entry_tmp;

/* In alpha a funcdef can be inside a function's scope so we needed to know if we are
 * inside a function 
 */
extern std::stack<struct st_entry*> func_stack;

static int loopcounter = 0;

void print_rules(std::string str) {
	 /* std::cout << "~ entered rule :\t " << str << std::endl; */
}

%}

%defines
%output "parser.cpp"
%define parse.error verbose
%code requires { #include<vector> }
%union {
	int intConst;
	double realConst;
	std::string *strConst;
	struct st_entry *st_entryVal;
	struct expr *expr_p;
	struct call *callVal;
	bool boolean;
	std::vector<expr*> *exprVec;
	struct stmt_t *stmt_pt;
	std::pair<expr*, expr*> *pairVal;
	std::vector<std::pair<expr*, expr*>* > *pairVec; 
	struct for_stmt *forVal; /* doesn't need to be pointer */
}

%token<intConst> INTEGER 
%token<realConst> REAL
%token<strConst> STRING
%token<strConst> ID
%token<boolean> TRUE FALSE

%token OR LOCAL NIL UMINUS MINUSMINUS
%token IF ELSE WHILE FUNCTION FOR RETURN BREAK CONTINUE AND NOT 
%token ASSIGN PLUS MINUS MULT DIVIDE PERCENT NOTEQUAL PLUSPLUS
%token GREATER LESSER GREATEREQUAL LESSEREQUAL EQUAL
%token LCBRACK RCBRACK LBRACK RBRACK LPAREN RPAREN SEMICOLON COMMA COLON COLONCOLON DOT DOTDOT

%type<expr_p> expr const assignexpr term primary ifprefix call
%type program
%type <expr_p>objectdef 
%type <intConst> idlist
%type <intConst> funcargs
%type <intConst> returnstmt
%type <exprVec> elist
%type <pairVec> indexed
%type <pairVal> indexedelem

%type<callVal> callsuffix
%type<callVal> normcall
%type<callVal> methodcall

%type <expr_p> tableitem;
%type <expr_p> member

%type <stmt_pt> stmts stmt

%type <intConst> whilestart whilecond
%type <stmt_pt> whilestmt

%type <stmt_pt> forstmt
%type <forVal> forprefix
%type <intConst> N

%type<intConst> M
%type <stmt_pt> ifstmt
%type<intConst> elseprefix

%type<stmt_pt> block

%type<st_entryVal> funcdef funcprefix
%type<strConst> funcname
%type<intConst> funcbody
%type<expr_p> lvalue
						  // Operator Tokens Hierarchy
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

program		: stmts						{	/* std::cout << "Finished reading statements\n"; */}
		 	|
			;

// Rule 2.
stmts		: stmts stmt 				{	resettemp();
											print_rules("2.1 stmts -> stmts stmt");
											if(!hasError) {
												if(!$2)
													$$ = NULL;
												else {
													$$ = new stmt_t();
													if(!loopcounter) {// solves some backpatch bug
														$$->breakList = mergelist($1->breakList, $2->breakList);
														$$->contList = mergelist($1->contList,  $2->contList);
													}else {
														patchlist($2->breakList, get_next_quad()+1);
														/* patchlist($2->contList,  arxh_tou_loop_pou_pi8anwn_ginetai_allou); */
													}
													if(func_stack.empty())
														$$->retList = mergelist($1->retList, $2->retList);
												}
											}
										}
			| stmt						{	print_rules("2.2 stmts -> ε");	$$ = $1;}
			
			;
// Rule 3.
stmt		: expr SEMICOLON			{	print_rules("3.1 stmt -> expr ;");
											$$ = new stmt_t();// new stmt_t calls make_stmt
	  										if( !$1 ) {
												$$ = NULL;
											}
											else if($1->type == BOOLEXPR_E) {
												backpatch($1->truelist, get_next_quad());
        										backpatch($1->falselist, get_next_quad() + 2);
												emit_branch_assign_quads($1);
											}
	  									}
			| ifstmt					{	print_rules("3.2 stmt -> ifstmt");
											$$ = $1;
										}
			| whilestmt					{	print_rules("3.3 stmt -> whilestmt");
											$$ = $1;
										}
			| forstmt					{	print_rules("3.4 stmt -> forstmt");
											$$ = $1;
										}
			| returnstmt				{	print_rules("3.5 stmt -> returnstmt");
											if(!hasError) {
												$$ = new stmt_t();
												$$->retList = newlist($1);
											}
										}
			| BREAK SEMICOLON			{	print_rules("3.6 stmt -> BREAK ;");			
											if(!loopcounter) {
											/* if(loop_stack.empty()) { */
												yyerror("break stmt outside loop has no use");
												$$ = NULL;
											}else {// we are in loop for sure
												$$ = new stmt_t();
												$$->breakList = get_current_quad();// newlist(get_next_quad();
												emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
											}
										}
			| CONTINUE SEMICOLON		{	print_rules("3.7 stmt -> CONTINUE ;");
											if(!loopcounter) {
											/* if(loop_stack.empty()) { */
												yyerror("continue stmt outside loop has no use");
												$$ = NULL;
											}else {// we are in loop for sure
												$$ = new stmt_t();
												$$->contList = newlist(get_current_quad());
												emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
											}
										}
			| block						{	print_rules("3.8 stmt -> block");
											$$ = $1;
										}
			| funcdef					{	print_rules("3.9 stmt -> funcdef");
											$$ = new stmt_t();
										}
			| SEMICOLON					{	print_rules("3.10 stmt -> ;");	$$ = NULL;
											// $$ = new stmt_t();
											$$ = NULL;
										}
			;

// Rule 4.
expr		: assignexpr				{	print_rules("4.1 expr -> assignexpr");
	  										$$ = $1;
	  									}
			| expr PLUS expr			{	print_rules("4.2 expr -> expr + expr");
											if($1 && $3) {
												$$ = expr_action_expr($1, ADD_OP, $3, exp_type_to_string($1) + " + " + exp_type_to_string($3));
											}
										}
			| expr MINUS expr			{	print_rules("4.3 expr -> expr - expr");
											if($1 && $3) {
												$$ = expr_action_expr($1, SUB_OP, $3, exp_type_to_string($1) + " - " + exp_type_to_string($3));
											}
										}
			| expr MULT expr			{	print_rules("4.4 expr -> expr * expr");
											if($1 && $3) {
												$$ = expr_action_expr($1, MUL_OP, $3, exp_type_to_string($1) + " * " + exp_type_to_string($3));
											}
										}
			| expr DIVIDE expr			{	print_rules("4.5 expr -> expr / expr");
											if($1 && $3) {
												$$ = expr_action_expr($1, DIV_OP, $3, exp_type_to_string($1) + " / " + exp_type_to_string($3));
											}
										}
			| expr PERCENT expr			{	print_rules("4.6 expr -> expr \% expr");
											if($1 && $3) {
												$$ = expr_action_expr($1, MOD_OP, $3, exp_type_to_string($1) + " % " + exp_type_to_string($3));
											}
										}
			| expr GREATER expr			{	print_rules("4.7 expr -> expr > expr");
											if($1 && $3) {
												bool is_arith = check_arith($1, "expr greater expr. Invalid use of comparison operator on non arithmetic type expression")
												&& check_arith($3, "expr greater expr. Invalid use of comparison operator on non arithmetic type expression");
												if(is_arith) {
													$$ = expr_compare_expr($1, IF_GREATER_OP, $3);
												}
											}
										}
			| expr GREATEREQUAL expr	{	print_rules("4.8 expr -> expr >= expr");
											if($1 && $3) {
												bool is_arith = check_arith($1, "expr greaterequal expr. Invalid use of comparison operator on non arithmetic type expression")
												&& check_arith($3, "expr greaterequal expr. Invalid use of comparison operator on non arithmetic type expression");
												if(is_arith) {
													$$ = expr_compare_expr($1, IF_GREATER_OP, $3);
												}
											}
										}
			| expr LESSER expr			{	print_rules("4.9 expr -> expr < expr");
											if($1 && $3) {
												bool is_arith = check_arith($1, "expr lesser expr. Invalid use of comparison operator on non arithmetic type expression")
												&& check_arith($3, "expr lesser expr. Invalid use of comparison operator on non arithmetic type expression");
												if(is_arith) {
													$$ = expr_compare_expr($1, IF_LESS_OP, $3);
												}
											}
										}
			| expr LESSEREQUAL expr		{	print_rules("4.10 expr -> expr <= expr");
											if($1 && $3) {
												bool is_arith = check_arith($1, "expr lessereq expr. Invalid use of comparison operator on non arithmetic type expression")
												&& check_arith($3, "expr lessereq expr. Invalid use of comparison operator on non arithmetic type expression");
												if(is_arith) {
													$$ = expr_compare_expr($1, IF_LESSEQ_OP, $3);
												}
											}
										}	
			| expr EQUAL expr			{	print_rules("4.11 expr -> expr == expr");
											if($1 && $3) {
												$$ = expr_compare_expr($1, IF_EQ_OP, $3);
											}
										}	
			| expr NOTEQUAL expr		{	print_rules("4.12 expr -> expr != expr");
											if($1 && $3) {
												$$ = expr_compare_expr($1, IF_NOTEQ_OP, $3);
											}
										}
			| expr AND {$1 = true_test($1);} M expr
										{	print_rules("4.13 expr -> expr AND expr");
											if($1 && $5) {
												expr *expr2 = true_test($5);
												backpatch($1->truelist, $4);
												union values val;// ginetai apotimhsh ths logikhs ekfrashs amesws kai apo8 sto BOOLEX quad
												val.boolConst = $1->value.boolConst && expr2->value.boolConst;

												$$ = new expr(BOOLEXPR_E, newtemp(), NULL, val);
												$$->truelist = expr2->truelist;
												$$->falselist = merge($1->falselist, expr2->falselist);
											}
										}
			| expr OR {$1 = true_test($1);} M expr
										{	print_rules("4.14 expr -> expr OR expr");
											if($1 && $5) {
												expr *expr2 = true_test($5);
												backpatch($1->falselist, $4);
												union values val;// ginetai apotimhsh ths logikhs ekfrashs amesws kai apo8 sto BOOLEX quad
												val.boolConst = $1->value.boolConst || expr2->value.boolConst;
												$$ = new expr(BOOLEXPR_E, newtemp(), NULL, val);
												$$->falselist = expr2->falselist;
												$$->truelist = merge($1->truelist, expr2->truelist);
											}
										}
			| term						{	print_rules("4.15 expr -> term");
											$$ = $1;
										}
			;

M 			:							{	
											print_rules("4+ M -> ε");
											$$ = get_next_quad();
										}
			;

// Rule 5.
term		: LPAREN expr RPAREN		{	
											print_rules("5.1 term -> ( expr )");
											$$ = $2;
	  									}
			| MINUS expr %prec UMINUS	{	
											print_rules("5.2 term -> - expr");
											if($2) {
												check_arith($2, "MINUS expr \%UMINUS");
												$$ = newexpr(ARITHEXPR_E);
												$$->sym = (istempexpr($2)) ? $2->sym : newtemp();
												emit(UMINUS_OP, $$, $2, NULL, get_next_quad(), yylineno);
											}
										}

			| NOT expr					{	
											print_rules("5.3 term -> NOT expr");
											if($2) {
												$$ = true_test($2);
												std::vector<unsigned> *tmp = $$->truelist;
												$$->truelist = $$->falselist;
												$$->falselist = tmp;
												$$->type = BOOLEXPR_E;// if not boolexpr it won't be patched
													backpatch($$->truelist, get_next_quad());
													backpatch($$->falselist, get_next_quad() + 2);
													/* expr *e = newexpr(BOOLEXPR_E); */
													/* e->sym = newtemp(); */
													/* emit_branch_assign_quads(e); */
												/* if($$->type != BOOLEXPR_E) { */
												/* 	backpatch($$->truelist, get_next_quad()); */
												/* 	backpatch($$->falselist, get_next_quad() + 2); */
												/* 	expr *e = newexpr(VAR_E); */
												/* 	e->sym = newtemp(); */
												/* 	emit_branch_assign_quads(e); */
												/* } */
											}
										}
			| PLUSPLUS lvalue			{
											print_rules("5.4 term -> ++ lvalue");
											if($2 && $2->sym->type == USER_FUNC || $2->sym->type == LIB_FUNC){
												yyerror("invalid assignment (lvalue is a function)");
											}
											if($2) {
												if($2->type == TABLEITEM_E) {
													$$ = emit_iftableitem($2);
													emit(ADD_OP, $$, newexpr_constint(1), $$, get_next_quad(), yylineno);
													emit(TABLESETELEM_OP, $2, $2->index, $$, get_next_quad(), yylineno);
												}
												else {
													emit(ADD_OP, $2, newexpr_constint(1), $2, get_next_quad(), yylineno);
													$$ = newexpr(ARITHEXPR_E);
													$$->sym = newtemp();
													emit(ASSIGN_OP, $$, $2, NULL, get_next_quad(), yylineno);
												}
											}
										}
			| lvalue PLUSPLUS			{
											print_rules("5.5 term -> lvalue ++");
											if($1 && $1->sym->type == USER_FUNC || $1->sym->type == LIB_FUNC){
												yyerror("invalid assignment (lvalue is a function)");
											}
											if ($1) {
												$$ = newexpr(VAR_E);
												$$->sym = newtemp();
												if($1->type == TABLEITEM_E) {
													expr *val = emit_iftableitem($1);
													emit(ASSIGN_OP, $$, val, NULL, get_next_quad(), yylineno);
													emit(ADD_OP, val, newexpr_constint(1), val, get_next_quad(), yylineno);
													emit(TABLESETELEM_OP, $1, $1->index, val, get_next_quad(), yylineno);
												}
												else {
													emit(ASSIGN_OP, $$, $1, NULL, get_next_quad(), yylineno);
													emit(ADD_OP, $1, newexpr_constint(1), $1, get_next_quad(), yylineno);
												}
											}
										}
			| MINUSMINUS lvalue			{
											print_rules("5.6 term -> -- lvalue");
											if($2 && $2->sym->type == USER_FUNC || $2->sym->type == LIB_FUNC){
												yyerror("invalid assignment (lvalue is a function)");
											}
											if($2) {
												if($2->type == TABLEITEM_E) {
													$$ = emit_iftableitem($2);
													emit(SUB_OP, $$, newexpr_constint(1), $$, get_next_quad(), yylineno);
													emit(TABLESETELEM_OP, $2, $2->index, $$, get_next_quad(), yylineno);
												}
												else {
													emit(SUB_OP, $2, newexpr_constint(1), $2, get_next_quad(), yylineno);
													$$ = newexpr(ARITHEXPR_E);
													$$->sym = newtemp();
													emit(ASSIGN_OP, $$, $2, NULL, get_next_quad(), yylineno);
												}
											}
										}
			| lvalue MINUSMINUS			{
											print_rules("5.7 term ->  lvalue --");
											if($1 && $1->sym->type == USER_FUNC || $1->sym->type == LIB_FUNC){
												yyerror("invalid assignment (lvalue is a function)");
											}
											if($1) {
												$$ = newexpr(VAR_E);
												$$->sym = newtemp();
												if($1->type == TABLEITEM_E) {
													expr *val = emit_iftableitem($1);
													emit(ASSIGN_OP, $$, val, NULL, get_next_quad(), yylineno);
													emit(SUB_OP, val, newexpr_constint(1), val, get_next_quad(), yylineno);
													emit(TABLESETELEM_OP, $1, $1->index, val, get_next_quad(), yylineno);
												}
												else {
													emit(ASSIGN_OP, $$, $1, NULL, get_next_quad(), yylineno);
													emit(SUB_OP, $1, newexpr_constint(1), $1, get_next_quad(), yylineno);
												}
											}
										}
			| primary					{	print_rules("5.8 term -> primary");
											$$ = $1;
										}
			;
// Rule 6.
assignexpr	: lvalue ASSIGN expr		{	print_rules("6.1 assignexpr -> lvalue = expr");
		   							 		if($1 && $1->sym && ($1->sym->type==LIB_FUNC || $1->sym->type==USER_FUNC) ) {
												yyerror("invalid assignment (lvalue is a function)");
											}else {
												if($1 && $1->type == TABLEITEM_E) {
													emit(TABLESETELEM_OP, $1, $1->index, $3, get_next_quad(), yylineno);
													$$ = emit_iftableitem($1);
                                        			$$->type = VAR_E;
												}
												else {
													if($3 && $1) {
														if($3->type == BOOLEXPR_E) {									
															backpatch($3->truelist, get_next_quad());
															backpatch($3->falselist, get_next_quad() + 2);
															emit_branch_assign_quads($3);
														}
														emit(ASSIGN_OP, $1, $3, NULL, get_next_quad(), yylineno);
														$$ = new expr(VAR_E, newtemp(), $3, $3->value);
														emit(ASSIGN_OP, $$, $1, NULL, get_next_quad(), yylineno);
													}
												}
											}
										}
			;

// Rule 7.
primary		: lvalue					{	
											print_rules("7.1 primary -> lvalue");
											if($1)						
												$$ = emit_iftableitem($1);
										}
			| call						{	
											print_rules("7.2 primary -> call");
											$$ = $1;
										}
			| objectdef					{	
											print_rules("7.3 primary -> objectdef");
											$$ = $1;
										}
			| LPAREN funcdef RPAREN		{	
											print_rules("7.4 primary -> ( funcdef )");
											expr *e = newexpr(PROGRAMFUNC_E);
											e->sym = $2;
											$$ = e;
										}
			| const						{	
											print_rules("7.5 primary -> const");
											$$ = $1;
										}
			;
// Rule 8.
lvalue		: ID						{	print_rules("8.1 lvalue -> ID");
											st_entry_tmp["r8"] = st_lookup(*$1);
											if(!st_entry_tmp["r8"]) {
												$$ = lvalue_expr (st_insert(*$1, (st_get_scope() == 0) ? GLOBAL_VAR : LOCAL_VAR));
											}else if( (st_entry_tmp["r8"]->type == LOCAL_VAR || (st_entry_tmp["r8"]->type == FORMAL_ARG
												&& st_entry_tmp["r8"]->scope != st_get_scope()))
													&& !func_stack.empty() && 
													(st_entry_tmp["r8"]->scope <= func_stack.top()->scope) ){
												yyerror("Cannot access local var \'"+*$1+"\' inside function \'"
												+func_stack.top()->name + "\'");
												$$ = NULL;
											}else {
												$$ = lvalue_expr (st_entry_tmp["r8"]);
											}
										}
			| LOCAL ID					{
											print_rules("8.2 lvalue -> local ID");
											st_entry_tmp["r8"] = st_lookup(*$2);
											if(st_entry_tmp["r8"] && (st_entry_tmp["r8"]->scope == st_get_scope()) ){
												$$ = lvalue_expr (st_entry_tmp["r8"]);
											}
											else if(st_entry_tmp["r8"] && st_entry_tmp["r8"]->type == LIB_FUNC){
												yyerror("variable \'" + *$2 + "\' shadows lib function");
												$$ = NULL;
											}
											else{
												$$ = lvalue_expr (st_insert(*$2, (st_get_scope() == 0) ? GLOBAL_VAR : LOCAL_VAR));
												// if(!func_stack.empty()) {
												// 	++func_stack.top()->totalLocals;													
												// }
											}
										}
			| COLONCOLON ID				{
											print_rules("8.3 lvalue -> ::ID");
											st_entry_tmp["r8"] = st_lookup(*$2, 0);
											if(!st_entry_tmp["r8"]){
												yyerror("No global variable "+*$2+" exists.");
												$$ = NULL;
												// assert(st_entry_tmp["r8"]);
											}
											else {
												$$ = lvalue_expr (st_entry_tmp["r8"]);
											}
										}
			| member					{
											print_rules("8.4 lvalue -> member");
											$$ = $1;
										}
			;
// Rule 8+.
tableitem	: lvalue DOT ID				{
											print_rules("8+.1 tableitem -> lvalue . ID");
											if($1)
												$$ = member_item($1, $3);
										}
			| lvalue LBRACK expr RBRACK	{
											print_rules("8+.2 tableitem -> lvalue [ expr ]");
											if($1 && $3) {
												expr *e = handle_bool_e($3);
												$1 = emit_iftableitem($1);
												$$ = newexpr(TABLEITEM_E);
												$$->sym = $1->sym;
												$$->index = e;
											}
										}


// Rule 9.
member		: tableitem					{
											print_rules("9.1 member -> tableitem");
											$$ = $1;
										}
			| call DOT ID				{
											print_rules("9.2 member -> call . ID");
											if($1)
												$$ = member_item($1,$3);
										}
			| call LBRACK expr RBRACK 	{
											print_rules("9.3 member -> call [ expr ]");
											if($3) {
												expr *e = handle_bool_e($3);
												$$ = e;
											}
										}
			;
// Rule 10.				
call		: call normcall				{	print_rules("10.1 member -> call ( elist )");
											if($1 && $2)
												$$ = make_call($1, $2->elist);
										}
			| lvalue callsuffix			{	print_rules("10.2 member -> lvalue callsuffix");
											if($1 && $2) {
												$1 = emit_iftableitem($1);
												if ($2->method){
													expr* t = $1;
													$1 = emit_iftableitem(member_item(t, $2->name));
													$2->elist->insert($2->elist->begin(),t); //v.insert(v.begin(), 6);
												}
												$$ = make_call($1, $2->elist);
											}
										}
			| LPAREN funcdef RPAREN LPAREN elist RPAREN
										{
											print_rules("10.3 member -> ( funcdef ) ( elist )");
											expr* func = newexpr(PROGRAMFUNC_E);
											func->sym = $2;
											$$ = make_call(func, $5);
										}
			;
// Rule 11.
callsuffix	: normcall					{	print_rules("11.1 member -> normcall");
											$$ = $1;
										}
			| methodcall				{	print_rules("11.2 member -> methodcall");
											$$ = $1;
										}
			;
// Rule 12.
normcall	: LPAREN elist RPAREN		{	print_rules("12.1 normcall -> ( elist )");
											$$ = new call(NULL, 0, $2);
									    }
/* 			| LPAREN RPAREN				{
											print_rules("12.2 normcall -> ( )");
										} */
			;
// Rule 13.
methodcall	: DOTDOT ID LPAREN elist RPAREN
										{
											print_rules("13.1 methodcall -> . . id ( elist )");
											$$ = new call($2, 1, $4);
									    }
			;
// Rule 14.
elist		: expr 						{
											print_rules("14.1 elist -> expr");
											$$ = new std::vector<expr*>;
											if($1) {
												$$->push_back(handle_bool_e($1));
											}
										}
			| elist COMMA expr 			{	
											print_rules("14.2 elist -> elist , expr");
											assert($1);
											if($3)
												$1->push_back(handle_bool_e($3));
										}
			|	/* ε */					{	
											print_rules("14.3 elist -> ε");
											$$ = new std::vector<expr*>;
										}
			;
// Rule 15.
objectdef	: LBRACK elist RBRACK 		{	
											print_rules("15.1 objectdef -> [ elist ]");
											expr* t = newexpr(NEWTABLE_E);
											t->sym = newtemp();
											emit(TABLECREATE_OP, t, NULL, NULL, get_next_quad(), yylineno);

											for(int i=0; i < ($2->size()); ++i){
												emit(
													TABLESETELEM_OP, 
													t, 
													newexpr_constint(i), 
													(*$2)[i], 
													get_next_quad(), yylineno);
											}
											$$ = t;
										}
			| LBRACK indexed RBRACK 	{	
											print_rules("15.2 objectdef -> [ indexed ]");
											expr* t = newexpr(NEWTABLE_E);
											t->sym = newtemp();
											emit(TABLECREATE_OP, t, NULL, NULL, get_next_quad(), yylineno);

											for(int i=0; i < ($2->size()); ++i){
												emit(
													TABLESETELEM_OP, 
													t, 
													(*$2)[i]->first, 
													(*$2)[i]->second, 
													get_next_quad(), yylineno);
											}
											$$ = t;
										}
			;
// Rule 16.
indexed		: indexedelem 				{	
											print_rules("16.1 indexed -> indexedelem");
											std::vector<std::pair<expr*, 
											expr*>* >* v = new std::vector<std::pair<expr*, 
											expr*>* >();
											v->push_back($1);	/*  pair, not *pair  */
											$$ = v;
										}
			| indexed COMMA indexedelem	{	
											print_rules("16.2 indexed ->  indexed , indexedelem");
											assert($1);
											$1->push_back($3);
											$$ = $1;  /* Praying that it calls deep copy ctor */
										}
			;
// Rule 17.
indexedelem	: LCBRACK expr COLON expr 	
			  RCBRACK					{	
				  							print_rules("17.1 indexedelem -> { expr : expr }");
											std::pair<expr*,expr*>* p = new std::pair<expr*,expr*>;
											if($2 && $4) {
												p->first = handle_bool_e($2);
												p->second = handle_bool_e($4);
											}
											$$ = p;	  
										}
			;
// Rule 18.
block		: LCBRACK 					{ 	print_rules("18.1 block -> { stmts }");
											st_increase_scope();
										}
	   		  stmts RCBRACK 			{
											if(!hasError)
												st_hide(st_get_scope());
											st_decrease_scope();
											
											$$ = $3;
										}
			| LCBRACK RCBRACK			{	print_rules("18.2 block -> { }");
											$$ = new stmt_t();
										}  // TODO if stmts rule can be empty this rule is redundant
			;
// Rule 19.
funcname    : ID						{
											$$ = $1;
											st_entry_tmp["r19"] = st_lookup(*$$);
											if((!st_entry_tmp["r19"]) ||
													( (st_entry_tmp["r19"]->scope < st_get_scope())
													&& (st_entry_tmp["r19"]->type != LIB_FUNC) ) ) {
												st_entry_tmp["r19"] = st_insert(*$$, USER_FUNC);
											}
											else {
												if(st_entry_tmp["r19"]->type == USER_FUNC) {// kseroume oti einai locally defined
													yyerror("redefinition of user function defined in line "
													+ std::to_string(st_entry_tmp["r19"]->line));
													/* $$ = NULL; */
												}
												else if(st_entry_tmp["r19"]->type == LIB_FUNC){
													yyerror("function definition shadows lib function");
													/* $$ = NULL; */
												}
												else if(st_entry_tmp["r19"]->type == LOCAL_VAR
														|| st_entry_tmp["r19"]->type == FORMAL_ARG
														|| st_entry_tmp["r19"]->type == GLOBAL_VAR){
													yyerror("variable \""+ *$$ + "\" already defined in line "
													+std::to_string(st_entry_tmp["r19"]->line));
													/* $$ = NULL; */
												}
												else {
													assert(0);
												}
												st_entry_tmp["r19"] = NULL;
											}
										}
			|							{
											$$ = new std::string();
											*$$ = st_godfather();
											st_insert(*$$, USER_FUNC);
										}
			;

funcprefix  : FUNCTION funcname			{
											print_rules("19.1 funcdef -> function ( idlist ) block");
											if($2) {
												$$ = st_lookup(*$2);
												assert($$);
												$$->totalLocals = 0;
												$$->iaddress = get_current_quad();
												st_entry_tmp["r19"] = $$;
												func_stack.push($$);
												expr *tmp_expr;
												emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
												tmp_expr = new expr(PROGRAMFUNC_E, $$, NULL, emptyval);
												pushscopeoffsetstack();
												emit(FUNCSTART_OP, tmp_expr, NULL, NULL, get_next_quad(), yylineno);
												st_increase_scope();
												enterscopespace();												
											}
										}
			;

funcargs:   LPAREN idlist RPAREN  		{
											$$ = $2;
											st_decrease_scope();
											enterscopespace();	
                                		}
            ;						
funcbody    : block						{
											if($1 && !hasError) {
												$$ = $1->retList;
											}
											exitscopespace();
										}
			;
funcdef		: funcprefix funcargs funcbody
		 								{	
											if(!hasError) {
												$1->totalLocals = getTotalLocals();
												$1->totalArgs = getTotalArgs();
												exitscopespace();
												popscopeoffsetstack();
												patchlist($3, get_next_quad());
												$$ = $1;
												expr *tmp_expr;
												tmp_expr = new expr(PROGRAMFUNC_E, $1, NULL, emptyval);
												emit(FUNCEND_OP, tmp_expr, NULL, NULL, get_next_quad(), yylineno);
												if(st_entry_tmp["r19"]) {
													func_stack.pop();
												}
												patchlabel($1->iaddress, get_next_quad());
											}
										}
			;
// Rule 20.
const		: INTEGER 					{	print_rules("20.1 const -> INTEGER");
											$$ = newexpr_constint($1);
										}	
	   		| REAL 						{	print_rules("20.2 const -> REAL");
											$$ = newexpr_constdouble($1);
										}
			| STRING 					{	print_rules("20.3 const -> STRING");
											$$ = newexpr_conststring($1);
										}
			| NIL 						{	print_rules("20.4 const -> NIL");
											$$ = newexpr(NIL_E);
										}
			| TRUE 						{	print_rules("20.5 const -> TRUE");
											$$ = newexpr_constbool(true);
										}
			| FALSE						{	print_rules("20.6 const -> FALSE");
											$$ = newexpr_constbool(false);
										}
			;
// Rule 21.
idlist		: ID 						{
											print_rules("22.1 idlist -> ID");
											st_entry_tmp["r21"] = st_lookup(*$1, 0);
											if(st_entry_tmp["r21"] && st_entry_tmp["r21"]->type ==  LIB_FUNC){
												yyerror("formal argument " + *$1 + " shadows lib func");
											}
											else {
												st_entry_tmp["r21"] = st_insert(*$1, FORMAL_ARG);
												// ++st_entry_tmp["r21"]->totalArgs;
												incformalArgOffset();
											}
											
											// $$ = poop
										}
			| idlist COMMA ID 			{
											print_rules("21.2 idlist -> idlist , ID");
											st_entry_tmp["r21"] = st_lookup(*$3);
											if(st_entry_tmp["r21"] && (st_entry_tmp["r21"]->type == FORMAL_ARG)
													&& st_entry_tmp["r21"]->scope == st_get_scope()){ // conflict
												yyerror("Argument "+ *$3 +" already exists.");
											}
											else if(st_entry_tmp["r21"] && st_entry_tmp["r21"]->type == LIB_FUNC){
												yyerror("formal argument " + *$3 + " shadows lib func");
											}
											else{
												st_entry_tmp["r21"] = st_insert(*$3, FORMAL_ARG);
												// ++st_entry_tmp["r21"]->totalArgs;
												incformalArgOffset();
											}
											// $$ = poop
										}
			|							{
											print_rules("21.3 empty id_list");
											// $$ = poop
										}
			;

// Rule 23.
ifprefix	: IF LPAREN expr RPAREN		{
											print_rules("23.1 ifprefix -> if ( expr )");
											if($3) {
												$3 = true_test($3);
												if($3->type == BOOLEXPR_E) {
													backpatch($3->truelist, get_next_quad());
													backpatch($3->falselist, get_next_quad() + 2);
													$3 = emit_branch_assign_quads($3);
													emit(IF_EQ_OP, NULL, $3, newexpr_constbool(true), get_next_quad() + 2, yylineno);
													emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
												}
												$3->falselist = new std::vector<unsigned>();
												$3->falselist->push_back(get_current_quad()-1);// pushback the jump so i can backpatch
												$$ = $3;
											}else
												$$ = NULL;
										}
			;

elseprefix	: ELSE						{
											print_rules("23.2 elseprefix -> else");
											$$ = get_current_quad();
											emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
										}
			;

ifstmt		: ifprefix stmt				{
											print_rules("23.3 ifstmt -> ifprefix stmt");
											if($1) {
												backpatch($1->falselist, get_next_quad());
											}
											$$ = $2;
										}
			| ifprefix stmt elseprefix stmt
										{
											print_rules("23.4 ifstmt -> ifstmt -> ifprefix stmt elseprefix stmt");
											if($1 && $2 && $4){
												backpatch($1->falselist, $3+2);
												patchlabel($3, get_next_quad());
												$2->breakList = mergelist($2->breakList, $4->breakList);
												$2->contList = mergelist($2->contList,  $4->contList);
												$2->retList = mergelist($2->retList, $4->retList);
												$$ = $2;
											}else {
												$$=NULL;
											}
											
										}
			;
			
// Rule 24.
whilestart	: WHILE						{	++loopcounter;	$$ = get_next_quad();}
		   	;
whilecond	: LPAREN expr RPAREN		{
											if($2) {
												$2 = true_test($2);
												if($2->type == BOOLEXPR_E) {
													$2 = handle_bool_e($2);
													emit(IF_EQ_OP, NULL, $2, newexpr_constbool(true), get_next_quad()+2, yylineno);
													$$ = get_current_quad();
													emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
												}else // case i have while(true) ...
													$$ = get_current_quad()-1;
												/* loop_stack.push(st_get_scope()); */
											}
										}
			;
whilestmt	: whilestart whilecond stmt
		  								{
											print_rules("24.1 whilestmt -> while ( expr ) stmt");
											if($3) {
												patchlist($3->breakList, get_next_quad()+1);
												patchlist($3->contList, $1);
												$$ = $3;
											}else
												$$ = new stmt_t();
											emit(JUMP_OP, NULL, NULL, NULL, $1, yylineno);
											patchlabel($2, get_next_quad());
											/* loop_stack.pop(); */
											--loopcounter;
										}
			;
// Rule 25.
forprefix	: FOR LPAREN elist SEMICOLON M expr SEMICOLON
										{
											print_rules("25.0 forprefix -> for ( elist ; expr ;");
											++loopcounter;
											/* loop_stack.push(st_get_scope()); */
											if($6) {
												$$ = new for_stmt();
												$$->test = $5;
												expr* e = handle_bool_e($6);
												$$->enter = get_current_quad();
												emit(IF_EQ_OP, NULL, e, newexpr_constbool(true), 0, yylineno);
											}else
												$$ = NULL;
										}
			;

forstmt		: forprefix N elist RPAREN N stmt N
										{
		 									print_rules("25.1 forstmt -> for ( elist ; expr ; elist ) stmt");
											patchlabel($1->enter, $5+2);
											patchlabel($2, get_next_quad());
											patchlabel($5, $1->test);
											patchlabel($7, $2 + 2);
											if($6){
												patchlist($6->breakList, get_next_quad());
												patchlist($6->contList, $2 + 2);
												$$ = $6;
											}else{
												$$ = NULL;
											}
											--loopcounter;
											/* loop_stack.pop(); */
										}
			;

N 			:							{
											$$ = get_current_quad();
                							emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
										}
			;

// Rule 26.
returnstmt 	: RETURN SEMICOLON 			{
											print_rules("26.1 returnstmt -> return ;");
											if (func_stack.empty()){
												yyerror("Use of 'return' while not in a function");
											}else {
												emit(RET_OP, NULL, NULL, NULL, get_next_quad(), yylineno);
												$$ = get_current_quad();// TODO check
												emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
											}
										}
			| RETURN expr SEMICOLON 	{
											print_rules("26.2 returnstmt -> return expr ;");
											if (func_stack.empty()){
												yyerror("Use of 'return' while not in a function");
											}else {
												emit(RET_OP, NULL, NULL, $2, get_next_quad(), yylineno);
												$$ = get_current_quad();// TODO check
												emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
												if($2 && $2->type == BOOLEXPR_E) {
													backpatch($2->truelist, get_next_quad());
													backpatch($2->falselist, get_next_quad() + 2);
													emit_branch_assign_quads($2);
												}
											}
										}
			;

%%

extern void validate_comments();

int yyerror(std:: string err) {
	std::cout << "\033[31m" << "ERROR " << "\033[37m" <<
	"in line " << yylineno << " : " << err << "\n";
	hasError = true;
	/* std::cout << "One or more errors on compilation, aborting... \n"; */
	/* exit(1); */
	return 1;
}


int main(int argc, char** argv) {
	std::cout << "\033[37m";// output is colored white
	std::string outname = "alpha.abc";
	int opt, num_files;
	FILE **files = NULL;

	while((opt = getopt(argc, argv, "o:i")) != -1){
		switch(opt){
			case 'i':
				outname = "";
				break;
			case 'o':
				outname = optarg;
				break;
			case '?':
				std::cout << "\033[33mTerminating\n";
			default:
				return 1;
		}
	}
	num_files = argc - optind;
	
	if(!num_files) {
		yyin = stdin;
	}
	else {
		files = (FILE**) malloc(num_files*sizeof(FILE*));
		for(int i = 0; optind < argc; ++optind, ++i) {
			if ( (files[i] = fopen(argv[optind], "r")) == NULL) {
				std::cout << argv[0] << ": Incorrect filename specified -- " << argv[optind] << std::endl;
				std::cout << "\033[33mTerminating\n";
				return 1;
			}
		}
	}
	validate_comments();
	st_initialize();
	if(num_files) {
		for ( int i = 0; i < num_files; ++i) {
			yyin = files[i];
			yyparse();
			if (hasError) {
				std::cout << argv[0] << ": One or more errors on compilation, aborting... \n";
				return 1;
			}
			fclose(files[i]);
		}
		st_print_table();
		generate();
		FILE *outf;
		if(outname != "") {
			outf = fopen(outname.c_str(), "w");
		}else {
			outf = stdout;
		}
		generate_binary_readable(outname);
		generate_binary(outf);
		print_quads();
		print_instructions();// for debug
	}else {
		yyparse();
		if (!hasError) {
			st_print_table();
			generate();
			FILE *outf;
			if(outname != "") {
				outf = fopen(outname.c_str(), "w");
			}else {
				outf = stdout;
			}
			generate_binary_readable(outname);
			generate_binary(outf);
			print_quads();
			print_instructions();
			/* avm_table* mytesttable;
			avm_memcell* tempcell;
			tempcell->type = STRING_M;
			tempcell->data.strVal = new std::string("Bab");
			avm_memcell* tempcell2;
			tempcell->type = STRING_M;
			tempcell->data.strVal = new std::string("Mpamphs");
			avm_tablesetelem(mytesttable, tempcell, tempcell2);
			libfunc_objectmemberkeys(*mytesttable); */
					
		} else {
			std::cout << "One or more errors on compilation, aborting... \n";
			return 1;
		}
	}
    return 0;
}
