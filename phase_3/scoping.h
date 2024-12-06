
#include <iostream>
#include <stack>
#include <assert.h>

enum scope_space
{
    programvar,
    formalarg,
    functionlocal
};
typedef enum scope_space scope_space;

void resetformalargsoffset(void);

void resetfunctionlocalsoffset(void);

scope_space currscopespace();

unsigned currscopeoffset(void);

void enterscopespace(void);

void exitscopespace(void);

void pushscopeoffsetstack(unsigned);

scope_space popscopeoffsetstack(void);

void restorecurrscopeoffset(unsigned n);

void incprogramVarOffset();

void incformalArgOffset();

void incfunctionLocalOffset();

bool scopeOffsetStackEmpty();

