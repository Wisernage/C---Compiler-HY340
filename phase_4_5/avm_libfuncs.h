#pragma once
#include <iostream>
#include "avm_table.h"

typedef void (*library_func_t)(void);

// void libfunc_print (void);

// void libfunc_typeof(void);

// void libfunc_input(void);

// void libfunc_objectmemberkeys(void);

// void libfunc_objecttotalmembers(void);

// void libfunc_objectcopy(void);

// void libfunc_totalarguments(void)

// void libfunc_argument(void);

void avm_calllibfunc(std::string);

void avm_init_libfuncs(void);