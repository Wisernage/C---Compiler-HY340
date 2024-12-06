#include "avm_mem_structs.h"
#include "avm_auxiliary.h"
#include "vm_elegance.h"

avm_memcell stack[AVM_STACKSIZE];

unsigned char   executionFinished = 0;
unsigned        pc = 0;
unsigned        currLine = 0;
unsigned        codeSize = 0;

avm_memcell reg_AX, reg_BX, reg_CX;
avm_memcell reg_RETVAL;
unsigned    top,topsp;

unsigned totalActuals = 0;

extern unsigned totalProgVars;

void avm_initstack (void) {
    for (size_t i = 0; i < AVM_STACKSIZE; ++i) {
        AVM_WIPEOUT(stack[i]);
		stack[i].type = UNDEF_M;
    }
    topsp = (N - 1) - totalProgVars;
    top = topsp;
}

extern unsigned char executionFinished;
void avm_dec_top (void) {
    if (!top) {
        avm_error("stack overflow");
        executionFinished = 1;
    }
    else
        --top;
}

void avm_push_envvalue (unsigned val) {
    stack[top].type         = NUMBER_M;
    stack[top].data.numVal  = val;
    avm_dec_top();
}

void avm_callsaveenvironment (void) {
    avm_push_envvalue(totalActuals);
    avm_push_envvalue(pc + 1);
    avm_push_envvalue(top + totalActuals + 2);
    avm_push_envvalue(topsp);
	topsp = top;
}

avm_memcell::avm_memcell() {
   memset( this, 0, sizeof( avm_memcell) );
   this->type = UNDEF_M;
}
avm_memcell::~avm_memcell() {
   memset( this, 0, sizeof( avm_memcell) );
}

avm_memcell::avm_memcell(avm_memcell_t type, void* data) {
    switch (type) {
        case NUMBER_M:
        {
            this->type = type;
            this->data.numVal = *(double*) data;
            break;
        }
        case STRING_M:
        {
            this->type = type;
            this->data.strVal = (std::string*) data;
            break;
        }
        case BOOL_M:
        {
            this->type = type;
            this->data.boolVal = *(bool*) data;
            break;
        }
        case TABLE_M:
        {
            this->type = type;
            this->data.tableVal = (avm_table*) data;
            break;
        }
        case USERFUNC_M:
        {
            this->type = type;
            this->data.funcVal = *(userfunc*) data;
            break;
        }
        case LIBFUNC_M:
        {
            this->type = type;
            this->data.libfuncVal = (std::string*) data;
            break;
        }
        case NIL_M:
        {
            this->type = type;
            /*this->~data();*/ // Maycause problems
            break;
        }
        case UNDEF_M:
        {
            this->type = type;
            /*this->~data();*/ // Maycause problems
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }
}

avm_memcell& avm_memcell::operator=(const avm_memcell& m) {
    switch (m.type) {
        case NUMBER_M:
        {
            this->type = m.type;
            this->data.numVal = m.data.numVal;
            break;
        }
        case STRING_M:
        {
            this->type = m.type;
            this->data.strVal = m.data.strVal;
            break;
        }
        case BOOL_M:
        {
            this->type = m.type;
            this->data.boolVal = m.data.boolVal;
            break;
        }
        case TABLE_M:
        {
            this->type = m.type;
            this->data.tableVal = m.data.tableVal;
            break;
        }
        case USERFUNC_M:
        {
            this->type = m.type;
            this->data.funcVal = m.data.funcVal;
            break;
        }
        case LIBFUNC_M:
        {
            this->type = m.type;
            this->data.libfuncVal = m.data.libfuncVal;
            break;
        }
        case NIL_M:
        {
            this->type = m.type;
            /*this->~data();*/ // Maycause problems
            break;
        }
        case UNDEF_M:
        {
            this->type = m.type;
            /*this->~data();*/ // Maycause problems
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }
    return (*this);
}
