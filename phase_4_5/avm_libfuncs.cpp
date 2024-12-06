#include "avm_libfuncs.h"
#include "avm_auxiliary.h"
#include "avm_mem_structs.h"
#include <cassert>
#include <cmath>

extern unsigned totalActuals;
extern avm_memcell reg_RETVAL;
extern unsigned topsp;
extern avm_memcell stack[AVM_STACKSIZE];
extern unsigned char   executionFinished;
extern unsigned        pc;
extern unsigned        currLine;
extern unsigned        codeSize;
extern instruction*    code;
extern unsigned    top, topsp;
extern unsigned totalProgVars;
extern int voidfuncstack;
std::unordered_map <std::string, library_func_t> libFuncHashTable;

void libfunc_print (void){
    unsigned n = avm_totalactuals();

    for(unsigned i = 0; i < n; ++i)
    {
        std::string s = avm_tostring(avm_getactual(i));
        std::cout << s /* << std::endl */;
    }
    voidfuncstack++;
}

int isNumber(const std::string& str)
{   
    bool dot = false;
    for (char const &c : str) {      
        if (c == '.' && dot == false){
            dot = true;
        }else if (std::isdigit(c) == 0){
            return 0;
        } 
    }
    if (dot){
        return 2;
    }
    return 1;
}

void handleString(std::string input){
    reg_RETVAL.type = STRING_M;
    reg_RETVAL.data.strVal = new std::string(input);
}

bool errorTable(avm_memcell t){
    if(t.type != TABLE_M){
		avm_error("Error, expected table type argument.");
        return true;
    }
    return false;
}

bool errorArg(std::string funcname){
    if(avm_totalactuals() != 1){
		avm_error("Error: one argument, (not "+ std::to_string(avm_totalactuals()) +") expected in '"+ funcname +"'!\n");
        return true;
    }
    return false;
}

bool errorArg2(std::string funcname){
    if(avm_totalactuals() != 0){
		avm_error("Error: zero arguments expected!\n");
        return true;
    }
    return false;
}

void libfunc_input(void)
{
    std::string input;
    std::cin >> input;
    int num;

    num = isNumber(input);

    if (input.back() == '"' && input[0] == '"'){
        handleString(input);
    }else if (num == 1 || num == 2){
        reg_RETVAL.type = NUMBER_M;
        reg_RETVAL.data.numVal = stod(input);
    }else if (input.find("true") != -1){
        reg_RETVAL.type = BOOL_M;
        reg_RETVAL.data.boolVal = 1;
    }else if(input.find("false") != -1){    /* double d std::to_string(d) */
        reg_RETVAL.type = BOOL_M;
        reg_RETVAL.data.boolVal = 0;
    }else if(input.find("nil") != -1){
        reg_RETVAL.type = NIL_M;
    }else{
        handleString(input);
    }
}

//void avm_tablesetelem(avm_table *table, avm_memcell* index, avm_memcell* content);

void libfunc_objectmemberkeys(void)
{
    avm_memcell *actual = avm_getactual(0);
	avm_table *actual_table = actual->data.tableVal;
    if (!errorTable(*actual)){

        avm_table* t = avm_tablenew();
        double i=0;
        for (auto x : *actual_table->strIndexed){
            avm_memcell *m = new avm_memcell();
            m->type = STRING_M;
            m->data.strVal = new std::string(x.first);
            t->numIndexed->insert(std::make_pair(i, *m));
            i++;
        }

        for (auto x : *actual_table->numIndexed){
            avm_memcell *m = new avm_memcell();
            m->type = NUMBER_M;
            m->data.numVal = x.first;
            t->numIndexed->insert(std::make_pair(i, *m));
            i++;
        }
        
        for (auto x : *actual_table->funcIndexed){
            avm_memcell *m = new avm_memcell();
            m->type = NUMBER_M;
            m->data.numVal = x.first;
            t->numIndexed->insert(std::make_pair(i, *m));
            i++;
        }
        for (auto x : *actual_table->trollIndexed){
            avm_memcell *m = new avm_memcell();
            m->type = STRING_M;
            m->data.strVal = new std::string(x.first);
            t->numIndexed->insert(std::make_pair(i, *m));
            i++;
        }
        avm_memcellclear(&reg_RETVAL);
        reg_RETVAL.type = TABLE_M;
        reg_RETVAL.data.tableVal = t;
    }
}

// void libfunc_objectmemberkeys(void)
// {
//     avm_memcell *actual = avm_getactual(0);
// 	avm_table *actual_table = actual->data.tableVal;
//     if (!errorTable(*actual)){
 
//         avm_table* t = avm_tablenew();
//         double i=0;
 
//         if (actual_table->numIndexed->size())
//         {
//             for (auto x : *(actual_table->numIndexed))
//             {
//                 std::string* s = new std::string(std::to_string(x.first));
//                 avm_memcell* content = new avm_memcell(STRING_M, s);
//                 delete s;
 
//                 double *cnt = new double(i);
//                 avm_memcell* index = new avm_memcell(NUMBER_M, cnt);
//                 delete cnt;
 
//                 avm_tablesetelem(t, index, content);
//                 ++i;
//             }
//         }
 
//         if (actual_table->strIndexed->size())
//         {
//             for (auto x : *(actual_table->strIndexed))
//             {
//                 std::string* s = new std::string(x.first);
//                 avm_memcell* content = new avm_memcell(STRING_M, s);
//                 delete s;
 
//                 double *cnt = new double(i);
//                 avm_memcell* index = new avm_memcell(NUMBER_M, cnt);
//                 delete cnt;
 
//                 avm_tablesetelem(t, index, content);
//                 ++i;
//             }
//         }
 
//         if (actual_table->funcIndexed->size())
//         {
//             for (auto x : *(actual_table->funcIndexed))
//             {
//                 std::string* s = new std::string(std::to_string(x.first));
//                 avm_memcell* content = new avm_memcell(STRING_M, s);
//                 delete s;
 
//                 double *cnt = new double(i);
//                 avm_memcell* index = new avm_memcell(NUMBER_M, cnt);
//                 delete cnt;
 
//                 avm_tablesetelem(t, index, content);
//                 ++i;
//             }
//         }
 
//         if (actual_table->trollIndexed->size())
//         {
//             for (auto x : *(actual_table->trollIndexed))
//             {
//                 std::string* s = new std::string(x.first);
//                 avm_memcell* content = new avm_memcell(STRING_M, s);
//                 delete s;
 
//                 double *cnt = new double(i);
//                 avm_memcell* index = new avm_memcell(NUMBER_M, cnt);
//                 delete cnt;
 
//                 avm_tablesetelem(t, index, content);
//                 ++i;
//             }
//         }
 
//         avm_memcellclear(&reg_RETVAL);
 
//         //reg_RETVAL.type = TABLE_M;
//         //reg_RETVAL.data.tableVal = t;
//         avm_memcell* ptr = new avm_memcell(TABLE_M, t);
//         avm_assign(&reg_RETVAL, ptr);
//     }
// }

void libfunc_objecttotalmembers(void){
    if(!errorArg("objecttotalmembers")){
        avm_memcell *actual = avm_getactual(0);
		avm_table *actual_table = actual->data.tableVal;
        if (!errorTable(*actual)){
            avm_memcellclear(&reg_RETVAL);
		    reg_RETVAL.type = NUMBER_M;
		    reg_RETVAL.data.numVal = actual_table->avm_table_elem_count();
        }
    }
}

void libfunc_objectcopy(void){
    avm_memcell *actual = avm_getactual(0);
	avm_table *actual_table = actual->data.tableVal;
    if (!errorTable(*actual)){
        avm_memcellclear(&reg_RETVAL);
	    reg_RETVAL.type = TABLE_M;
        reg_RETVAL.data.tableVal = actual_table;
    }
}   

void libfunc_totalarguments(void){
    unsigned int p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    avm_memcellclear(&reg_RETVAL);
    if(p_topsp + totalProgVars + 1 == AVM_STACKSIZE)
    {
        avm_error("'totalarguments' called outside a function.\n");
        reg_RETVAL.type = NIL_M;
    }
    else{
        reg_RETVAL.type = NUMBER_M;
        reg_RETVAL.data.numVal = avm_totalactuals();
    }
}

void libfunc_argument(void) {
    if (!errorArg("argument")) {
        unsigned int p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
        avm_memcellclear(&reg_RETVAL);
        if(!p_topsp)
        {
            avm_error("'argument' called outside a function.\n");
            reg_RETVAL.type = NIL_M;
        }
        else {
            if (avm_getactual(0)->type != NUMBER_M){
                if ((int)avm_getactual(0)->data.numVal <= avm_totalactuals()){
                    avm_assign(&reg_RETVAL, &stack[p_topsp + AVM_STACKENV_SIZE + (int)avm_getactual(0)->data.numVal]);
                }else{
                    avm_error("Argument index exceeds total arguments in 'argument' library function call.\n");
                }
            }else{
                avm_error("Argument of library function 'argument' is not a valid number, thus cannot be used as index.\n");
            }
        }
    } 
}

void libfunc_typeof(void)
{
   if(!errorArg("typeof")){
    	avm_memcellclear(&reg_RETVAL);
		reg_RETVAL.type = STRING_M;
		reg_RETVAL.data.strVal = new std::string(typeStrings[avm_getactual(0)->type]);
   }
}

void libfunc_strtonum(void){
    if (!errorArg("strtonum")){
        if(avm_getactual(0)->type != STRING_M){
			avm_error("Error: strtonum argument '" + avm_tostring(avm_getactual(0)) + "'is not a string.");
		}else{
            avm_memcellclear(&reg_RETVAL);
            if (isNumber(*avm_getactual(0)->data.strVal) == 0){
                reg_RETVAL.type = NIL_M;
            }else{
                reg_RETVAL.type = NUMBER_M;
                reg_RETVAL.data.numVal = stod(*avm_getactual(0)->data.strVal);
            }
        }
    }
}

void libfunc_sqrt(void)
{
	if (!errorArg("sqrt")){
        if (avm_getactual(0)->type == NUMBER_M){
            avm_memcellclear(&reg_RETVAL);
            double sqrtv = std::sqrt(avm_getactual(0)->data.numVal);
            if(std::isnan(sqrtv)){
                reg_RETVAL.type = NIL_M;
            }else{
                reg_RETVAL.type = NUMBER_M;
                reg_RETVAL.data.numVal = sqrtv;
            }
        }else{
            avm_error("Error: sqrt argument '" + avm_tostring(avm_getactual(0)) + "'is not a number.");
        }
	}
}

void libfunc_cos(void)
{
	if (!errorArg("cos")){
        if (avm_getactual(0)->type == NUMBER_M){
            avm_memcellclear(&reg_RETVAL);
            double cosv = std::cos(avm_getactual(0)->data.numVal);
            if(std::isnan(cosv)){
                reg_RETVAL.type = NIL_M;
            }else{
                reg_RETVAL.type = NUMBER_M;
                reg_RETVAL.data.numVal = cosv;
            }
        }else{
            avm_error("Error: sqrt argument '" + avm_tostring(avm_getactual(0)) + "'is not a number.");
        }
	}
}

void libfunc_sin(void)
{
	if (!errorArg("sin")){
        if (avm_getactual(0)->type == NUMBER_M){
            avm_memcellclear(&reg_RETVAL);
            double sinv = std::sin(avm_getactual(0)->data.numVal);
            if(std::isnan(sinv)){
                reg_RETVAL.type = NIL_M;
            }else{
                reg_RETVAL.type = NUMBER_M;
                reg_RETVAL.data.numVal = sinv;
            }
        }else{
            avm_error("Error: sqrt argument '" + avm_tostring(avm_getactual(0)) + "'is not a number.");
        }
	}
}


library_func_t avm_getlibraryfunc(std::string id)
{
	if (libFuncHashTable.find(id) == libFuncHashTable.end()) {// not found
        return NULL;
    }
    return libFuncHashTable[id];
}

void avm_calllibfunc(std::string id) {
    library_func_t f = avm_getlibraryfunc(id);
    if(!f) {
        avm_error("Unsupported library function " + id + " called!");
        executionFinished = 1;
    }
    else {
        topsp = top;
		(*f)();
		if(!executionFinished){
			execute_funcexit((instruction * )0);
        }
        totalActuals = 0;
    }
    
}

void avm_init_libfuncs (void) {
    libFuncHashTable["print"] = libfunc_print;
    libFuncHashTable["input"] = libfunc_input;
    libFuncHashTable["objectmemberkeys"] = libfunc_objectmemberkeys;
    libFuncHashTable["objecttotalmembers"] = libfunc_objecttotalmembers;
    libFuncHashTable["objectcopy"] = libfunc_objectcopy;
    libFuncHashTable["totalarguments"] = libfunc_totalarguments;
    libFuncHashTable["typeof"] = libfunc_typeof;
    libFuncHashTable["strtonum"] = libfunc_strtonum;
    libFuncHashTable["sqrt"] = libfunc_sqrt;
    libFuncHashTable["cos"] = libfunc_cos;
    libFuncHashTable["sin"] = libfunc_sin;
}
