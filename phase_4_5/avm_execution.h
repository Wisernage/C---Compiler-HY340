#pragma once
#include "avm_structures.h"
#include "avm_mem_structs.h"
#include "avm_table.h"
#include "avm_auxiliary.h"
#include "avm_libfuncs.h"



double  consts_getnumber (unsigned index);
std::string consts_getstring (unsigned index);
std::string libfuncs_getused (unsigned index);

avm_memcell* avm_translate_operand (vmarg* arg, avm_memcell* reg);

#define N 4096
#define AVM_MAX_INSTRUCTIONS  NOP_V
#define AVM_NUMACTUALS_OFFSET   +4
#define AVM_SAVEDPC_OFFSET      +3
#define AVM_SAVEDTOP_OFFSET     +2
#define AVM_SAVEDTOPSP_OFFSET   +1


void execute_assign (instruction*);
void execute_add (instruction*);
void execute_sub (instruction*);
void execute_mul (instruction*);
void execute_div (instruction*);
void execute_mod (instruction*);

/* the following funcs are not needed but exist for possible dispatcher assignments */
void execute_uminus (instruction*);
void execute_and (instruction*);
void execute_or (instruction*);
void execute_not (instruction*);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void execute_jeq (instruction*);
void execute_jne (instruction*);
void execute_jle (instruction*);
void execute_jge (instruction*);
void execute_jlt (instruction*);
void execute_jgt (instruction*);
void execute_call (instruction*);
void execute_pusharg (instruction*);
void execute_funcenter (instruction*);
void execute_funcexit (instruction*);
void execute_newtable (instruction*);
void execute_tablegetelem (instruction*);
void execute_tablesetelem (instruction*);
void execute_jump(instruction*);
void execute_nop (instruction*);

typedef void (*execute_func_t)(instruction*);

#define AVM_ENDING_PC codeSize

void execute_cycle (void);

void avm_assign (avm_memcell* lv, avm_memcell* rv);

void execute_assign (instruction* instr);

unsigned avm_totalactuals(void);

avm_memcell* avm_getactual (unsigned i);

unsigned avm_get_envvalue(unsigned i);

// void avm_registerlibfunc (char* id, library_func_t addr);

void execute_call (instruction* instr);

void execute_funcenter (instruction* instr);

void execute_funcexit (instruction* unused);
void execute_pusharg (instruction* instr);

/* arithmetics */
typedef double (*arithmetic_func_t) (double x, double y);

double add_impl (double x, double y);
double sub_impl (double x, double y);
double mul_impl (double x, double y);
double div_impl (double x, double y);
double mod_impl (double x, double y);


#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

void execute_arithmetic (instruction* inst);

/* relationals */
bool avm_compare_jeq(avm_memcell* rv1,avm_memcell* rv2);
bool avm_compare_jne(avm_memcell* rv1,avm_memcell* rv2);
bool avm_compare_jle(avm_memcell* rv1,avm_memcell* rv2);
bool avm_compare_jlt(avm_memcell* rv1,avm_memcell* rv2);
bool avm_compare_jge(avm_memcell* rv1,avm_memcell* rv2);
bool avm_compare_jgt(avm_memcell* rv1,avm_memcell* rv2);

typedef bool (*cmp_func_t) (avm_memcell* rv1,avm_memcell* rv2);

/* dispatcher for comparison funcs */
void execute_comparison (instruction* instr);

#define execute_jeq execute_comparison
#define execute_jne execute_comparison
#define execute_jle execute_comparison
#define execute_jge execute_comparison
#define execute_jlt execute_comparison
#define execute_jgt execute_comparison

/* tables execution instrs */

extern avm_memcell*    avm_tablegetelem (
                                    avm_table* table,
                                    avm_memcell* index
                                 );
extern void            avm_tablesetelem (
                                    avm_table*  table,
                                    avm_memcell* index,
                                    avm_memcell* content
                                 );
/* already declared above ^ */                                
/* void            execute_newtable (instruction* instr);
void            execute_tablegetelem (instruction* instr);
void            execute_tablesetelem (instruction* instr); */
