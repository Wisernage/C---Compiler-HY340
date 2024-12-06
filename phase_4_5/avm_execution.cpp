#include "avm_execution.h"
#include "avm_auxiliary.h"
#include "avm_mem_structs.h"
#include "avm_table.h"

extern avm_memcell stack[AVM_STACKSIZE];
extern avm_memcell reg_AX, reg_BX, reg_CX;
extern avm_memcell reg_RETVAL;
extern unsigned    top, topsp, totalActuals;

extern std::vector<double>          numConsts;
extern std::vector<std::string> 	stringConsts;
extern std::vector<std::string> 	namedLibFuncs;
extern std::vector<userfunc> 		userFuncs;

extern unsigned char   executionFinished;
extern unsigned        pc;
extern unsigned        currLine;
extern unsigned        codeSize;
extern instruction*    code;
extern int voidfuncstack;

avm_memcell* avm_translate_operand (vmarg* arg, avm_memcell* reg) {
    switch (arg->type) {

        /* Variables */
        case GLOBAL_A:  return &stack[AVM_STACKSIZE - 1 - arg->val];// diavazei apo global scope (prwto stack frame)
        case LOCAL_A:   return &stack[topsp - arg->val];// diavazei apo to top stack frame
        case FORMAL_A:  return &stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val];// diavazei apo to top stac frame

        case RETVAL_A: return &reg_RETVAL;

        case NUMBER_A: {
            reg->type = NUMBER_M;
            reg->data.numVal = consts_getnumber(arg->val);
            return reg;
        }

        case STRING_A: {
            reg->type = STRING_M;
            reg->data.strVal = new std::string(consts_getstring(arg->val));
            return reg;
        }

        case BOOL_A: {
            reg->type = BOOL_M;
            reg->data.boolVal = arg->val;
            return reg;
        }

        case NIL_A: reg->type = NIL_M; return reg;

        case USERFUNC_A: {
            reg->type = USERFUNC_M;
            reg->data.funcVal.address = userFuncs[arg->val].address;
            reg->data.funcVal.localSize = userFuncs[arg->val].localSize;
            reg->data.funcVal.argSize = userFuncs[arg->val].argSize;
            return reg;
        }

        case LIBFUNC_A: {
            reg->type = LIBFUNC_M;
            reg->data.libfuncVal = new std::string((libfuncs_getused(arg->val)));
            return reg;
        }

        default: assert(0);
    }
}

double  consts_getnumber (unsigned index) {
    return numConsts[index];
}

std::string consts_getstring (unsigned index) {
    return stringConsts[index];
}

std::string libfuncs_getused (unsigned index) {
    return namedLibFuncs[index];
}

void execute_jump(instruction* instr) {
	assert(instr);
	pc = instr->result.val;

}

void execute_nop (instruction* i) {
    std::cout << "!!! executed nop! !!!\n";
    assert(0);
}

execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
    execute_nop,// uminus
    execute_nop,// and
    execute_nop,// or
    execute_nop,// not
    execute_jeq,
	execute_jne,
    execute_jle,
    execute_jge,
    execute_jlt,
    execute_jgt,
    execute_call,
    execute_pusharg,
/*     execute_ret,
    execute_getretval, */
    execute_funcenter,
    execute_funcexit,
    execute_newtable,
    execute_tablegetelem,
    execute_tablesetelem,
    execute_jump,
    execute_nop
};

cmp_func_t comparisonFuncs[] = {
    avm_compare_jeq,
    avm_compare_jne,
    avm_compare_jle,
    avm_compare_jge,
    avm_compare_jlt,
    avm_compare_jgt
};

arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};



void execute_cycle (void) {
    if (executionFinished)
        return;
    else 
    if (pc == AVM_ENDING_PC) {
        executionFinished = 1;
        return;
    }
    else {
        assert(pc < AVM_ENDING_PC);

        instruction* instr = code + pc;
        assert(
            instr->opcode >= 0 &&
            instr->opcode <= AVM_MAX_INSTRUCTIONS
        );
        if (instr->srcLine)
            currLine = instr->srcLine;
        unsigned oldPC = pc;
        (*executeFuncs[instr->opcode]) (instr);
        if (pc == oldPC)
            ++pc;
    }
}

void avm_assign (avm_memcell* lv, avm_memcell* rv) {
    if (voidfuncstack){
        rv->type = NIL_M;
        voidfuncstack--;
    }
    if (lv == rv)       /* Same cells? destructive tro assign! */
        return;

    if (lv->type == TABLE_M &&  /* Same tables? no need to assign. */
			rv->type == TABLE_M &&
			lv->data.tableVal == rv->data.tableVal) {
        return;
	}

    if (rv->type == UNDEF_M) {   /* From undefined r-value? warn! */
        avm_warning("assigning from 'undef' content!");
	}
    avm_memcellclear(lv);   /* Clear old cell contents */

    //memccpy(lv, rv, sizeof(avm_memcell));   /* In C++ dispatch instead */
    *lv = *rv;

    /* Now take care of copied values or reference counting */
    if (lv->type == STRING_M)
        lv->data.strVal = new std::string(*(rv->data.strVal));
    else if (lv->type == TABLE_M)
        avm_tableincrefcounter(lv->data.tableVal);

}

void execute_assign (instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*) 0);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &reg_AX);

    assert(lv && (&stack[N-1] >= lv && &stack[top] || lv == &reg_RETVAL)); // care - changed!
    assert(rv);/* should do similar assertion test here */ // added semicolon

    avm_assign(lv, rv);
}

unsigned avm_totalactuals(void) {
    return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell* avm_getactual (unsigned i){
    assert(i < avm_totalactuals());
    return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

unsigned avm_get_envvalue(unsigned i) {
    assert(stack[i].type == NUMBER_M);
    unsigned val = (unsigned)stack[i].data.numVal;
    assert(stack[i].data.numVal == val);
    return val;
}

void execute_call (instruction* instr) {
    avm_memcell* func = avm_translate_operand(&instr->result, &reg_AX);
    assert(func);
	// func->data.funcVal.address = instr->result.val;// svhsimo
    avm_callsaveenvironment();

    switch (func->type) {
        case USERFUNC_M: {
            pc = func->data.funcVal.address;
            assert(pc < AVM_ENDING_PC);
            assert(code[pc].opcode == FUNCENTER_V);
            break;
        }
        
        case STRING_M: {
            avm_calllibfunc(namedLibFuncs[instr->result.val]);
			break;
        }

        case LIBFUNC_M: {
            avm_calllibfunc(namedLibFuncs[instr->result.val]);
			break;
        } 
        
        default: {
            std::string* s = new std::string(avm_tostring(func));
            avm_error("call: cannot bind "+ *s +" to function!\n");
            delete s;
            executionFinished = 1;
        }
        
    }
}

// userfunc* avm_get_funcinfo (unsigned address) {       
//      return &userFuncs[code[address].result.val];
// }

void execute_funcenter (instruction* instr) {
    avm_memcell* func = avm_translate_operand(&instr->result, &reg_AX);
    assert(func);
    assert(pc == func->data.funcVal.address); /* Func addr should match PC */

    /* Callee actions here. */
	if (func->type == LIBFUNC_M && namedLibFuncs[(code[pc].result.val)] != "totalarguments"){
        totalActuals = 0;
    }
    // userfunc* funcInfo = &func->data.funcVal;
    topsp = top;
    top = top - func->data.funcVal.localSize;

}

void execute_funcexit (instruction* unused) {
    unsigned oldTop = top;
    top     = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
    pc      = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
    topsp   = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    totalActuals = (top == topsp) ? 0 : avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
    while (++oldTop <= top) /* Intentionally ignering first. */
        avm_memcellclear(&stack[oldTop]);
}

void execute_pusharg (instruction* instr) {
    avm_memcell* arg = avm_translate_operand(&instr->arg1, &reg_AX);
    assert(arg);

    /*  This is actually stack[top] = arg, but we have to
        use avm_assign
     */
    avm_assign(&stack[top], arg);
    ++totalActuals;
    avm_dec_top();
}

double add_impl (double x, double y) { return x + y; }
double sub_impl (double x, double y) { return x - y; }
double mul_impl (double x, double y) { return x * y; }
double div_impl (double x, double y) 
{
    assert(y != 0);
    return x / y; 
}
double mod_impl (double x, double y) 
{ 
    return ((int) x) % ((int) y);   /* no further error checking */ 
}

void execute_arithmetic (instruction* instr) {
    avm_memcell* lv  = avm_translate_operand(&instr->result, (avm_memcell*) 0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &reg_AX); 
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &reg_BX);

    assert(lv && (&stack[N-1] >= lv && lv > &stack[top] || lv==&reg_RETVAL));
    assert(rv1 && rv2);

    if (rv1->type != NUMBER_M || rv2->type != NUMBER_M) {
        avm_error("not a number in arithmetic!");
        executionFinished = 1;
    }
    else {
        arithmetic_func_t op = arithmeticFuncs[instr->opcode - ADD_V];
        avm_memcellclear(lv);
        lv->type           = NUMBER_M;
        lv->data.numVal = (*op) (rv1->data.numVal, rv2->data.numVal);
    }
}

/* relationals */
bool avm_compare_jeq(avm_memcell* rv1,avm_memcell* rv2) {
    assert(rv1 && rv2 && (rv1->type == rv2->type));

    switch (rv1->type) {
        case NUMBER_M:
            return (rv1->data.numVal - rv2->data.numVal) ? false : true;
        
        case STRING_M:
            return (rv1->data.strVal->compare(*(rv2->data.strVal))) ? false : true;
        
        case BOOL_M:
            return (rv1->data.boolVal == rv2->data.boolVal);
        
        case TABLE_M:
            return (rv1->data.tableVal == rv2->data.tableVal);
        
        case USERFUNC_M:
            return (rv1->data.funcVal.address == rv2->data.funcVal.address);
        
        case LIBFUNC_M:
            return (rv1->data.libfuncVal->compare(*(rv2->data.libfuncVal))) ? false : true;
        
        case NIL_M:
            return true;
    
        case UNDEF_M:
            return false;
        
        default:
            assert(0);
    }
}

bool avm_compare_jne(avm_memcell* rv1,avm_memcell* rv2) {
    return !avm_compare_jeq(rv1, rv2);
}

bool avm_compare_jle(avm_memcell* rv1,avm_memcell* rv2) {
    assert(rv1 && rv2 && (rv1->type == rv2->type) && rv1->type == NUMBER_M);
    return (avm_get_numVal(rv1) <= avm_get_numVal(rv2));
}

bool avm_compare_jlt(avm_memcell* rv1,avm_memcell* rv2) {
    assert(rv1 && rv2 && (rv1->type == rv2->type) && rv1->type == NUMBER_M);
    return (avm_get_numVal(rv1) < avm_get_numVal(rv2));
}

bool avm_compare_jge(avm_memcell* rv1,avm_memcell* rv2) {
    assert(rv1 && rv2 && (rv1->type == rv2->type) && rv1->type == NUMBER_M);
    return (avm_get_numVal(rv1) >= avm_get_numVal(rv2));
}

bool avm_compare_jgt(avm_memcell* rv1,avm_memcell* rv2) {
    assert(rv1 && rv2 && (rv1->type == rv2->type) && rv1->type == NUMBER_M);
    return (avm_get_numVal(rv1) > avm_get_numVal(rv2));
}

/*  Comparison includes all cmp jmp instr s 
 *  JEQ , JNE
 *  JGT , JLT
 *  JGE , JLE   except plain JMP
 */
void execute_comparison (instruction* instr) {
    assert(instr->result.type == LABEL_A);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &reg_AX);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &reg_BX);

    assert(rv1 && rv2); /* Maybe more checks needed */

    bool res;

    if (rv1->type == UNDEF_M || rv2->type == UNDEF_M) {
        avm_error("comparison involving UNDEF type not allowed.");
        executionFinished = 1;
    }
    else if (rv1->type == NIL_M || rv2->type == NIL_M) {
		res = (rv1->type == NIL_M && rv2->type == NIL_M);
	}
    else if (rv1->type == BOOL_M || rv2->type == BOOL_M) {
		res = (avm_tobool(rv1) == avm_tobool(rv2));
	}
    else if(rv1->type != rv2->type) {
        avm_error("comparison between different types not allowed.");
        executionFinished = 1;
    }
    else
        res = comparisonFuncs[instr->opcode - JEQ_V](rv1, rv2);

    if(res && !executionFinished)
        pc = instr->result.val;

}

/* table instrs */

void            execute_newtable (instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell* ) 0);
    assert(lv && (&stack[N - 1] >= lv && lv > &stack[top] || lv==&reg_RETVAL));

    avm_memcellclear(lv);

    lv->type        = TABLE_M;
    lv->data.tableVal       = avm_tablenew();
    avm_tableincrefcounter(lv->data.tableVal);
}

void            execute_tablegetelem (instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell* ) 0);
    avm_memcell* t = avm_translate_operand(&instr->arg1, (avm_memcell* ) 0);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &reg_AX);

    assert(lv && &stack[N-1] >= lv && lv > &stack[top] || lv==&reg_RETVAL);
    assert(t && &stack[N-1] >= t && t > &stack[top]);
    assert(i);

    avm_memcellclear(lv);
    lv->type = NIL_M;   /* Default value */

    if (t->type != TABLE_M) 
        avm_error("Illegal use of type "+ typeStrings[t->type] +" as table!");
    else
    {
        avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);
        if (content->type != UNDEF_M)   /* Difference from Savvidis */
            avm_assign(lv, content);
        else
        {
            std::string ts = avm_tostring(t);
            std::string is = avm_tostring(i);
            avm_warning(ts+"["+is+"]"+" not found!");
        }
    }
    

}
void execute_tablesetelem (instruction* instr) {
    avm_memcell* t  = avm_translate_operand(&instr->result, (avm_memcell*) 0);    
    avm_memcell* i  = avm_translate_operand(&instr->arg1, &reg_AX);
    avm_memcell* c  = avm_translate_operand(&instr->arg2, &reg_BX);

    assert(t && &stack[N-1] >= t && t > &stack[top]);
    assert(i && c);

    if (t->type != TABLE_M)
        avm_error("Illegal use of type "+ typeStrings[t->type] +" as table!");
    else
        avm_tablesetelem(t->data.tableVal, i, c);
}
