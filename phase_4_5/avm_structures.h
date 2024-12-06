#pragma once
#include <iostream>
#include "quads.h"
#define MAGIC_NUM 340200501

enum vmopcode {
    ASSIGN_V,       ADD_V,          SUB_V,
    MUL_V,          DIV_V,          MOD_V,
    UMINUS_V,       AND_V,          OR_V,
    NOT_V,          JEQ_V,          JNE_V,
    JLE_V,          JGE_V,          JLT_V,
    JGT_V,          CALL_V,         PUSHARG_V,
    FUNCENTER_V,    FUNCEXIT_V,     NEWTABLE_V,
    TABLEGETELEM_V, TABLESETELEM_V, JUMP_V,
	NOP_V
};
enum vmarg_t {
    LABEL_A,
    GLOBAL_A,
    FORMAL_A,
    LOCAL_A,
    NUMBER_A,
    STRING_A,
    BOOL_A,
    NIL_A,
    USERFUNC_A,
    LIBFUNC_A,
    RETVAL_A,
	UNDEF_A
};

typedef struct vmarg {
    vmarg_t     type;
    unsigned 	val;
}vmarg;

struct instruction {
    vmopcode    opcode;
    vmarg       result;
    vmarg       arg1;
    vmarg       arg2;
    unsigned    srcLine;
};

struct userfunc {
    unsigned    address;
    unsigned    localSize;
    unsigned    argSize;
    std::string       *id;
};

void make_operand(expr* e, vmarg *arg);

void generate_ASSIGN(quad*);
void generate_ADD(quad*);
void generate_SUB(quad*);
void generate_MUL(quad*);
void generate_DIV(quad*);
void generate_MOD(quad*);
void generate_NOP(quad*);// UMINUS i-code is handled with MUL -1 
void generate_AND(quad*);
void generate_OR(quad*);
void generate_NOT(quad*);
void generate_IF_EQ(quad*);
void generate_IF_NOTEQ(quad*);
void generate_IF_LESSEQ(quad*);
void generate_IF_GREATEREQ(quad*);
void generate_IF_LESS(quad*);
void generate_IF_GREATER(quad*);
void generate_CALL(quad*);
void generate_PARAM(quad*);
void generate_RET(quad*);
void generate_GETRETVAL(quad*);
void generate_FUNCSTART(quad*);
void generate_FUNCEND(quad*);
void generate_TABLECREATE(quad*);
void generate_TABLEGETELEM(quad*);
void generate_TABLESETELEM(quad*);
void generate_JUMP(quad*);

typedef void (*generator_func_t) (quad*);// generator_func_p is a pointer to func with param quad* ret/ing void

void print_instructions ();
void generate();
void generate_binary_readable (std::string outname);
void generate_binary(FILE *outf);
