#ifndef SYMTABLE_H
#define SYMTABLE_H


#include <iostream>
#include <ostream>
#include <assert.h>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <stack>

enum st_entry_type {
	GLOBAL_VAR = 0,
	FORMAL_ARG,
	LOCAL_VAR,
	LIB_FUNC,
	USER_FUNC
};

struct st_entry {
	bool active;
	std::string name;
	enum st_entry_type type;
	std::vector<st_entry*> *argList;	// Formal Arguments list
										// in case of function
	unsigned int scope;
	int line;
} typedef st_entry;

st_entry* st_insert(std::string name_input, enum st_entry_type type_input);

std::string st_godfather();

/* increases scope */
void st_increase_scope();

/* decreases scope */
void st_decrease_scope();

/* self explainatory */
unsigned int st_get_scope();

/* 	
*  	performs search in scope order order high to low every scope as long as the stored
*  	vars are active (active = scope > 0 && not hidden by RCBRACK'}')
*/
st_entry* st_lookup(std::string);

/* searches for active var_ids in given scope */
st_entry* st_lookup(std::string name_input, unsigned int scope_input);

int st_hide(unsigned int scope_input);

int load_2_arglist(struct st_entry* arg);// inserts arg to st_entry

int offload_arglist(st_entry* func);// gives the arg_list ptr to the func

/* void st_set_inloop(bool b); */

/* bool st_get_inloop(); */

void st_initialize();// also pushes the lib func entries in scope 0 of st

void st_print_table();

/* void st_freeAll(); */

/* a search to the current arg list while picking the formal arguments to check for
 * conflicts among the arg list names*/
st_entry *check_arglist(std::string name_input);

#endif


