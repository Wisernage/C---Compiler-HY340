#include "scoping.h"

unsigned programVarOffset = 0;
unsigned formalArgOffset = 0;
unsigned functionLocalOffset = 0;
unsigned curScopeSpace = 0;
std::stack<function_offsets> scopeoffsetstack;
function_offsets funcoffsets;


void resetformalargsoffset(void){
	formalArgOffset = 0;
}

void resetfunctionlocalsoffset(void){
	functionLocalOffset = 0;
}

void resetcurScopeSpace(void){
	curScopeSpace = 0;
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

unsigned currscopeoffset(){
    switch(currscopespace())
    {
	    case programvar:	return programVarOffset;
	    case functionlocal:	return functionLocalOffset; 
	    case formalarg:		return formalArgOffset; 
        default:            assert(0);
    }
}

// void restorecurrscopeoffset(unsigned n){
//     switch (currscopespace())
//     {
//         case programvar:        programVarOffset = n;       break;
//         case functionlocal:     functionLocalOffset = n;    break;
//         case formalarg:         formalArgOffset = n;        break;
//         default: assert(0);

//     }
// } MHTSOS these are not used now

void enterscopespace(void){
	curScopeSpace++;
}

void exitscopespace(void){
	assert(curScopeSpace > 0);
	curScopeSpace--;
}

unsigned returncurrentspace(){
    return curScopeSpace;
}

void updateFuncOffsets(){
    funcoffsets.curScopeSpace = curScopeSpace;
    funcoffsets.formalArgOffset = formalArgOffset;
    funcoffsets.functionLocalOffset = functionLocalOffset;
}

void updateOffsetsFromStackTop(){
    curScopeSpace = scopeoffsetstack.top().curScopeSpace;
    formalArgOffset = scopeoffsetstack.top().formalArgOffset;
    functionLocalOffset = scopeoffsetstack.top().functionLocalOffset;
}

void pushscopeoffsetstack(){
    updateFuncOffsets();
	scopeoffsetstack.push(funcoffsets);
    resetformalargsoffset();
    resetfunctionlocalsoffset();
    resetcurScopeSpace();
}

void popscopeoffsetstack(){
    updateOffsetsFromStackTop();
	scopeoffsetstack.pop();
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

unsigned getTotalLocals(){
    return functionLocalOffset;
}

unsigned getTotalArgs(){
    return formalArgOffset;
}