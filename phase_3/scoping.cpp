#include "scoping.h"

unsigned programVarOffset = 0;
unsigned formalArgOffset = 0;
unsigned functionLocalOffset = 0;
unsigned curScopeSpace = 0;
std::stack<unsigned> scopeoffsetstack;


void resetformalargsoffset(void){
	formalArgOffset = 0;
}

void resetfunctionlocalsoffset(void) {
	functionLocalOffset = 0;
}

scope_space currscopespace(void){
    if(curScopeSpace == 0){
        return programvar;
	} else if(curScopeSpace == 1){
        return formalarg;
	} else{
        return functionlocal;
	}
}

unsigned currscopeoffset() {
    switch(currscopespace())
    {
	    case programvar:	return programVarOffset;
	    case functionlocal:	return functionLocalOffset; 
	    case formalarg:		return formalArgOffset; 
        default:            assert(0);
    }
}

void restorecurrscopeoffset(unsigned n){
    switch (currscopespace())
    {
        case programvar:        programVarOffset = n;       break;
        case functionlocal:     functionLocalOffset = n;    break;
        case formalarg:         formalArgOffset = n;        break;
        default: assert(0);

    }
}

void enterscopespace(void){
	curScopeSpace++;
}

void exitscopespace(void){
	assert(curScopeSpace > 0);
	curScopeSpace--;
}

void pushscopeoffsetstack(unsigned n){
	scopeoffsetstack.push(n);
}

scope_space popscopeoffsetstack(){
	unsigned offset = scopeoffsetstack.top();
	scopeoffsetstack.pop();
	if(offset == 0){
        return programvar;
	} else if(offset % 2 == 1){
        return formalarg;
	} else {
        return functionlocal;
	}
}

void incprogramVarOffset(){
    programVarOffset++;
}

void incformalArgOffset(){
    formalArgOffset++;
}

void incfunctionLocalOffset(){
    functionLocalOffset++;
}

bool scopeOffsetStackEmpty(){
	return scopeoffsetstack.empty();
}

