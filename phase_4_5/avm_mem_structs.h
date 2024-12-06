#pragma once
#include <iostream>
#include <cstring>
#include "avm_structures.h"

enum avm_memcell_t {
        NUMBER_M    =0,
        STRING_M    =1,
        BOOL_M,
        TABLE_M,
        USERFUNC_M,
        LIBFUNC_M,
        NIL_M,
        UNDEF_M
};

// #pragma pack(push)
// #pragma pack(1)

typedef struct avm_memcell {
    avm_memcell_t   type;
    union data {
        double              numVal;
        std::string        *strVal;
        bool                boolVal;
        struct avm_table   *tableVal;
        userfunc            funcVal;    /* changed from unsigned */
        std::string        *libfuncVal;
        data() {memset( this, 0, sizeof( data ) );}
        ~data() {memset( this, 0, sizeof( data ) );}
    }data;
    avm_memcell();
    ~avm_memcell();
    avm_memcell(avm_memcell_t type, void* data);
    avm_memcell& operator=(const avm_memcell& m);
} avm_memcell;


#define AVM_STACKSIZE   4096
#define AVM_WIPEOUT(m)  memset(&(m), 0, sizeof(m))
#define AVM_STACKENV_SIZE   4// unused

void avm_initstack (void);

void avm_dec_top (void);
void avm_push_envvalue (unsigned val);
void avm_callsaveenvironment (void);

// #pragma pack(pop)
