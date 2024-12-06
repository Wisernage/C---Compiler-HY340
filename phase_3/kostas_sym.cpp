#include "symtable.h"
#include "scoping.h"
#include <unordered_map>
#include <cassert>
unsigned int scope;
unsigned int scope_MAX;
unsigned in_funcdef;
unsigned int nonos;

extern int yylineno;

std::vector<std::unordered_map<std::string, std::vector<st_entry>>> symbol_table;
std::vector<struct st_entry *> f_arg_list;
std::stack<struct st_entry *> func_stack;

st_entry *st_insert(std::string name, enum st_entry_type type) {
	st_entry *new_entry;
	assert(new_entry = new st_entry);
	*new_entry = {true,  name, type,
		// (type == USER_FUNC) ? new std::vector<st_entry > : NULL,
		scope, (unsigned)yylineno};
	symbol_table[scope][name].push_back(*new_entry);// works even if bucket~key==name is empty
	return new_entry;
}

std::string st_godfather() {
	std::string s = "$f" + std::to_string(nonos++);
	return s;
}

void st_increase_scope() {
	scope++;
	if (scope > scope_MAX) {
		scope_MAX = scope;
		symbol_table.push_back(*(new std::unordered_map<std::string, std::vector<st_entry>>));
	}
}

void st_decrease_scope() { scope--; }

unsigned int st_get_scope() { return scope; }

/* a search to the current arg list while picking the formal arguments to check
 * for conflicts among the arg list names*/
st_entry *check_arglist(std::string name_input) {
	for (int i = 0; i < f_arg_list.size(); ++i) {
		if ((f_arg_list[i]->name == name_input) &&
				(f_arg_list[i]->active == true)) {
			return f_arg_list[i];
		}
	}
	return NULL;
}

st_entry *st_lookup(std::string name_input) {
	/* checking all current and lower scopes */
	for (int i = st_get_scope(); i >= 0; i--) {
		if(symbol_table[i].find(name_input) != symbol_table[i].end()) {
			for(int j = 0; j < symbol_table[i][name_input].size(); ++j) {
				if(symbol_table[i][name_input][j].active == true
						&& name_input == symbol_table[i][name_input][j].name) {
					return &symbol_table[i][name_input][j];
				}
			}
		}
	}
	return NULL;
}

// for local purpose
st_entry *st_lookup(std::string name_input, unsigned int scope_input) {
	st_entry *tmp = NULL;
		if(symbol_table[scope_input].find(name_input) != symbol_table[scope_input].end()){
			for(int j = 0; j < symbol_table[scope_input][name_input].size(); ++j){
				if(symbol_table[scope_input][name_input][j].active == true
						&& name_input == symbol_table[scope_input][name_input][j].name){
					return &symbol_table[scope_input][name_input][j];
				}
			}
		}
	return tmp;
}

int st_hide(unsigned int scope_input) {
	assert(scope_input > 0);
	for (auto map_pair : symbol_table[scope_input]) {
		map_pair.second.end()->active = false;// putania??Only last entry of vec is to be checked
	}
	return 0;
}

int load_2_arglist(struct st_entry *arg) {
	f_arg_list.push_back(arg);
	return 0;
}

int offload_arglist(st_entry *func) {
	assert(func->type == USER_FUNC);
	// *(func->argList) = f_arg_list;
	f_arg_list.clear();
	return 0;
}

void st_initialize() {
	scope = 0;
	yylineno = 1;
	scope_MAX = 0;
	nonos = 0;
	symbol_table.push_back(std::unordered_map<std::string, std::vector<st_entry>>());
	st_insert("print", LIB_FUNC);
	st_insert("input", LIB_FUNC);
	st_insert("objectmemberkeys", LIB_FUNC);
	st_insert("objecttotalmembers", LIB_FUNC);
	st_insert("objectcopy", LIB_FUNC);
	st_insert("totalarguments", LIB_FUNC);
	st_insert("argument", LIB_FUNC);
	st_insert("typeof", LIB_FUNC);
	st_insert("strtonum", LIB_FUNC);
	st_insert("sqrt", LIB_FUNC);
	st_insert("cos", LIB_FUNC);
	st_insert("sin", LIB_FUNC);
}

void st_print_table() {
	std::string st_type_print[] = {"global variable", "formal argument",
		"local variable", "library function",
		"user function"};
	std::cout << "\n";
	int i = 0;
	for (auto map : symbol_table) {
		std::cout << " -----------      scope #" << i++ << "      ----------- "
			<< std::endl;
		for (auto pair : map) {
			for (int j = 0; j < pair.second.size(); j++){
				std::cout << "\"" << pair.first << "\" [" << st_type_print[pair.second[j].type] << "] "
				<< "(line " << pair.second[j].line << ") (scope " << pair.second[j].scope << ")\n";
			}
		}
		std::cout << std::endl;
	}
	std::cout << " ------------------------------------------- \n";
	std::cout << "\"name\"  [type]   (line)  (scope)   \n";
}
