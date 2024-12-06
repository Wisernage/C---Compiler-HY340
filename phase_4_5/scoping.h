#pragma once
#include <iostream>
#include <stack>
#include <assert.h>

enum scope_space
{
    programvar,
    functionlocal,
    formalarg
};

typedef enum scope_space scope_space;

struct function_offsets
{
    unsigned formalArgOffset = 0;
    unsigned functionLocalOffset = 0;
    unsigned curScopeSpace = 0;
} typedef function_offsets;

void resetformalargsoffset(void);

void resetfunctionlocalsoffset(void);

scope_space currscopespace();

unsigned currscopeoffset(void);

void enterscopespace(void);

void exitscopespace(void);

void pushscopeoffsetstack(void);

void restorecurrscopeoffset(void);

void popscopeoffsetstack(void);

unsigned getTotalLocals();

void incprogramVarOffset();

void incformalArgOffset();

void incfunctionLocalOffset();

bool scopeOffsetStackEmpty();

unsigned returncurrentspace();

unsigned getTotalArgs();