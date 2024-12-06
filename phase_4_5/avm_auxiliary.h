#pragma once
#include "avm_mem_structs.h"
#include "avm_structures.h"
#include "avm_table.h"
#include "avm_execution.h"

typedef void (*memclear_func_t) (avm_memcell*);

void memclear_string (avm_memcell* m);
void memclear_table (avm_memcell* m);

void avm_memcellclear (avm_memcell* m);

void avm_warning (std::string s, ...);
void avm_error (std::string s, ...);

double avm_get_numVal(avm_memcell* m);
std::string avm_get_stringVal(avm_memcell* m);
bool avm_get_boolVal(avm_memcell* m);
userfunc avm_get_funcVal(avm_memcell* m);

extern std::string typeStrings[];

std::string avm_tostring (avm_memcell* m);

typedef std::string (*tostring_func_t) (avm_memcell*);

std::string number_tostring (avm_memcell* m);
std::string string_tostring (avm_memcell* m);
std::string bool_tostring (avm_memcell* m);
std::string table_tostring (avm_memcell* m);
std::string userfunc_tostring (avm_memcell* m);
std::string libfunc_tostring (avm_memcell* m);
std::string nil_tostring (avm_memcell* m);
std::string undef_tostring (avm_memcell* m);

userfunc* avm_get_funcinfo (unsigned address);

/* bool convertors */
typedef bool (*tobool_func_t) (avm_memcell*);


bool number_tobool (avm_memcell* m);
bool string_tobool (avm_memcell* m);
bool bool_tobool (avm_memcell* m);
bool table_tobool (avm_memcell* m);
bool userfunc_tobool (avm_memcell* m);
bool libfunc_tobool (avm_memcell* m);
bool nil_tobool (avm_memcell* m);
bool undef_tobool (avm_memcell* m);

bool avm_tobool (avm_memcell* m);
