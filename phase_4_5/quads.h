#pragma once
#include "symtable.h"

enum iopcode {
	ASSIGN_OP,		ADD_OP,			SUB_OP,
	MUL_OP,			DIV_OP,			MOD_OP,
	UMINUS_OP,		AND_OP,			OR_OP,
	NOT_OP,			IF_EQ_OP,		IF_NOTEQ_OP,
	IF_LESSEQ_OP,	IF_GREATEREQ_OP,IF_LESS_OP,
	IF_GREATER_OP,	CALL_OP,		PARAM_OP,
	RET_OP,			GETRETVAL_OP,	FUNCSTART_OP,
	FUNCEND_OP,		TABLECREATE_OP,	TABLEGETELEM_OP,
	TABLESETELEM_OP,JUMP_OP
};

enum expr_t {
	VAR_E,			TABLEITEM_E,	PROGRAMFUNC_E,
	LIBRARYFUNC_E,	ARITHEXPR_E,	BOOLEXPR_E,
	ASSIGNEXPR_E,	NEWTABLE_E,		CONSTINT_E,
	CONSTSTRING_E,	CONSTDOUBLE_E,	CONSTBOOL_E,
	NIL_E
};

union values {
	int intConst;
	double doubleConst;
	std::string *strConst;
	bool boolConst;
};

struct expr {
	expr_t type;
	st_entry *sym;
	expr *index;
	union values value;
	std::vector<unsigned> *truelist;
	std::vector<unsigned> *falselist;
	expr();
	expr(expr_t type, st_entry *sym, expr *index, union values value);
}typedef expr;

typedef struct quad {
	iopcode op;
	expr *result;
	expr *arg1;
 	expr *arg2;
	unsigned label;
	unsigned line;
	unsigned taddress;
}quad;

typedef struct call {
	std::vector<expr*> *elist;
	unsigned char method;
	std::string *name;
	call(std::string *name, unsigned char method, std::vector<expr*> *elist);
}call;

typedef struct for_stmt {
    int test;
    int enter;
}for_stmt;

typedef struct stmt_t {
	int breakList, contList, retList;
	stmt_t();
} stmt_t;

void make_stmt (stmt_t* s);

int newlist (int i);

void patchlist (int list, int label);

int mergelist (int l1, int l2);

void emit(iopcode op, expr *result, expr *arg1, expr *arg2, unsigned label,
		unsigned line);

void print_quads();

unsigned int get_current_quad();

unsigned int get_next_quad();

void print_line();
