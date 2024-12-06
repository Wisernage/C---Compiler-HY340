#include "avm_table.h"
#include "avm_auxiliary.h"
#include "avm_execution.h"
#include "avm_mem_structs.h"
#include <assert.h>
#include <string>
#include <unordered_map>    /// NAi comple f

avm_table::avm_table ()
{
    this->strIndexed = new std::unordered_map<std::string,avm_memcell>;
    this->numIndexed = new std::unordered_map<double,avm_memcell>;
    this->funcIndexed = new std::unordered_map<unsigned,avm_memcell>;
    this->trollIndexed = new std::unordered_map<std::string,avm_memcell>;
    this->avm_table_elem_count();
}

avm_table::~avm_table () 
{
    delete this->strIndexed;
    delete this->numIndexed;
    delete this->funcIndexed; 
    delete this->trollIndexed;
} 

void avm_tableincrefcounter (avm_table* t) {
    ++t->refCounter;
}

void avm_tabledecrefcounter (avm_table* t) {
    assert(t->refCounter > 0);
    if(!--t->refCounter)
        avm_tabledestroy(t);
}


avm_table* avm_tablenew (void) {
    avm_table* t = new avm_table();
    t->refCounter = 0; /* total becomes set inside ctor */   
    // AVM_WIPEOUT(*t);
    // I think it's unneeded now!
    return t;
}


void avm_tabledestroy (avm_table* t)
{
    t->~avm_table();
    
}

unsigned avm_table::avm_table_elem_count()
{
    unsigned res = this->strIndexed->size() + this->numIndexed->size() +
                this->funcIndexed->size() + this->trollIndexed->size();
    this->total = res;
    return res;
}

void memclear_table (avm_memcell* m)
{
    assert(m->data.tableVal);
    avm_tabledecrefcounter(m->data.tableVal);
}

avm_memcell*    avm_tablegetelem (
                                    avm_table* table,
                                    avm_memcell* index
                                 )
{
    assert(table && index);

    switch (index->type)
    {
        case NUMBER_M:
        {
            std::unordered_map<double,avm_memcell>::const_iterator got =
                table->numIndexed->find (index->data.numVal);
            if (got == table->numIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->numIndexed->at(index->data.numVal));
        }
        case STRING_M:          
        {
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->strIndexed->find (*(index->data.strVal));
            if (got == table->strIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->strIndexed->at((*(index->data.strVal))));
        }
        case USERFUNC_M:
        {
            std::unordered_map<unsigned,avm_memcell>::const_iterator got =
                table->funcIndexed->find (index->data.funcVal.address);
            if (got == table->funcIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->funcIndexed->at(index->data.funcVal.address));  
        }
        case BOOL_M:
        {
            bool b = index->data.boolVal;
            std::string s = (b) ? "true" : "false";
            
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->trollIndexed->find (s);
            if (got == table->trollIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->trollIndexed->at(s));   
        }
        case LIBFUNC_M:
        {
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->trollIndexed->find (*(index->data.libfuncVal));
            if (got == table->trollIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->trollIndexed->at((*(index->data.libfuncVal))));
        }
        case NIL_M:
        {
            // We ignore this
            /* std::string s = "nil";
            
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->trollIndexed->find (s);
            if (got == table->trollIndexed->end())
            {
                return new avm_memcell();
            }
            else
            return &(table->trollIndexed->at(s));  */
            return new avm_memcell();
        }
        case UNDEF_M:
        {
            avm_error("Element cannot be cannot have UNDEF type!");
            return new avm_memcell();   // Added to pacify compiler :/
            break;
        }

        default:
            assert(0);
            return new avm_memcell(); // Added to pacify compiler :/
            break;
    }
}

void            avm_tablesetelem (
                                    avm_table*  table,
                                    avm_memcell* index,
                                    avm_memcell* content
                                 )
{
    assert(table && index && content);
    assert(content->type != UNDEF_M);

    switch (index->type)
    {
        case NUMBER_M:
        {
            // Following check happens in execute assign so unneeded!
            /* if ((*(table->numIndexed))[index->data.numVal].type == TABLE_M) 
            {
                avm_tabledecrefcounter((*(table->numIndexed))[index->data.numVal].data.tableVal);
            } */
                
            /* check if exists and if it does clear memcell */
            std::unordered_map<double,avm_memcell>::const_iterator got =
                table->numIndexed->find (index->data.numVal);
            if (!(got == table->numIndexed->end()))
                avm_memcellclear(&(table->numIndexed->at(index->data.numVal)));

            /* then procced with assignment */
            avm_memcell* m = new avm_memcell();
            avm_assign(m, content); 
            (*(table->numIndexed))[index->data.numVal] = *m;
            table->avm_table_elem_count();
            // Following check happens in execute assign so unneeded!
            /* if (content->type == TABLE_M)
            {
                avm_tableincrefcounter(content->data.tableVal);
            } */
            break;
        }
        case STRING_M:          
        {
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->strIndexed->find (*(index->data.strVal));
            if (got == table->strIndexed->end())
                avm_memcellclear(&(table->strIndexed->at((*(index->data.strVal)))));

            avm_memcell* m = new avm_memcell();
            avm_assign(m, content);
            (*(table->strIndexed))[*(index->data.strVal)] = *m;
            table->avm_table_elem_count();
            break;
        }
        case USERFUNC_M:
        {
            std::unordered_map<unsigned,avm_memcell>::const_iterator got =
                table->funcIndexed->find (index->data.funcVal.address);
            if (got == table->funcIndexed->end())
                avm_memcellclear(&(table->funcIndexed->at(index->data.funcVal.address)));  

            avm_memcell* m = new avm_memcell();
            avm_assign(m, content);
            (*(table->funcIndexed))[index->data.funcVal.address] = *m;
            table->avm_table_elem_count();
            break;
        }
        case BOOL_M:
        {
            bool b = index->data.boolVal;
            std::string s = (b) ? "true" : "false";

            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->trollIndexed->find (s);
            if (got == table->trollIndexed->end())
                avm_memcellclear(&(table->trollIndexed->at(s)));
            
            avm_memcell* m = new avm_memcell();
            avm_assign(m, content);
            (*(table->trollIndexed))[s] = *m;
            table->avm_table_elem_count();
            break;
        }
        case LIBFUNC_M:
        {
            std::unordered_map<std::string,avm_memcell>::const_iterator got =
                table->trollIndexed->find (*(index->data.libfuncVal));
            if (got == table->trollIndexed->end())
                avm_memcellclear(&(table->trollIndexed->at((*(index->data.libfuncVal)))));
            
            avm_memcell* m = new avm_memcell();
            avm_assign(m, content);
            (*(table->trollIndexed))[*(index->data.libfuncVal)] = *m;
            table->avm_table_elem_count();
            break;
        }
        case NIL_M:
        {
            // We just ignore insertion!
            /* std::string s = "nil";
            (*(table->trollIndexed))[s] = *content;
            table->avm_table_elem_count(); */
            break;
        }
        case UNDEF_M:
        {
            avm_error("Cannot hash UNDEF type to table element!");
            break;
        }

        default:
            assert(0);
            break;
    }
}
