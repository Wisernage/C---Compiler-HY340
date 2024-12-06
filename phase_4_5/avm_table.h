#pragma once
#include <unordered_map>
#include "avm_mem_structs.h"
#include "avm_auxiliary.h"

avm_table*      avm_tablenew (void);
void            avm_tabledestroy (avm_table* t);

/* typedef struct avm_table_bucket {
    avm_memcell         key;
    avm_memcell         value;
    avm_table_bucket*   next;
}; // Savvidis impl */

#define AVM_TABLE_HASHSIZE 211

typedef struct avm_table {
    unsigned                        refCounter;
    /* no pointers! */
    std::unordered_map<std::string,
    avm_memcell>                    *strIndexed;
    std::unordered_map<double,
    avm_memcell>                    *numIndexed;
    std::unordered_map<unsigned,
    avm_memcell>                    *funcIndexed;
    std::unordered_map<std::string,
    avm_memcell>                    *trollIndexed; /* allows keys of type : libfunc, bool, nil */
    unsigned        total; /* total no of elems. !! Use method instead*/
	avm_table();
    ~avm_table();
    unsigned avm_table_elem_count();
} avm_table;

void            avm_tableincrefcounter (avm_table* t);
void            avm_tabledecrefcounter (avm_table* t);
/* void            avm_tablebucketsinit (avm_table_bucket** p); // Savvidis impl */

void memclear_table (avm_memcell* m);

/* could have been class methods but choose to do it savvidis way */
avm_memcell*    avm_tablegetelem (
                                    avm_table* table,
                                    avm_memcell* index
                                 );

void            avm_tablesetelem (
                                    avm_table*  table,
                                    avm_memcell* index,
                                    avm_memcell* content
                                 );
