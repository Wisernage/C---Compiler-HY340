#include "symtable.h"

unsigned scope;
unsigned scope_MAX;
unsigned in_funcdef;
bool in_loop;
unsigned nonos;

extern int yylineno;

std::vector<std::vector<st_entry>> symbol_table;
std::vector<struct st_entry *> f_arg_list;
std::stack<struct st_entry *> func_stack;

st_entry *st_insert(std::string name, enum st_entry_type type) {
	st_entry *new_entry;
	new_entry = new st_entry{true,  name, type,
		scope, (unsigned)yylineno};
	assert(new_entry);
	symbol_table[scope].push_back(*new_entry);
	return new_entry;
}

std::string st_godfather() {
	std::string s = "$" + std::to_string(nonos++);
	return s;
}

void st_increase_scope() {
	scope++;
	if (scope > scope_MAX) {
		scope_MAX = scope;
		symbol_table.push_back(*(new std::vector<st_entry>));
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
	st_entry *tmp = NULL;
	/* checking all current and lower scopes */
	for (int i = (static_cast<int>(st_get_scope())); i >= 0; i--) {
		for (int j = 0; j < symbol_table[i].size(); ++j) {
			if ((symbol_table[i][j].active) &&
					(symbol_table[i][j].name == name_input)) {
				return &symbol_table[i][j];
			}
		}
	}
	return tmp;
}

// for local purpose
st_entry *st_lookup(std::string name_input, unsigned int scope_input) {
	st_entry *tmp = NULL;
	for (int i = 0; i < symbol_table[scope_input].size(); i++) {
		if ((symbol_table[scope_input][i].active) &&
				(symbol_table[scope_input][i].name == name_input)) {
			tmp = &symbol_table[scope_input][i];
		}
	}
	return tmp;
}

int st_hide(unsigned int scope_input) {
	assert(scope_input > 0);
	for (int i = 0; i < symbol_table[scope_input].size(); ++i) {
		symbol_table[scope_input][i].active = false;
	}
	return 0;
}

// int load_2_arglist(struct st_entry *arg) {
// 	f_arg_list.push_back(arg);
// 	return 0;
// }

// int offload_arglist(st_entry *func) {
// 	assert(func->type == USER_FUNC);
// 	assert(func->argList);
// 	*(func->argList) = f_arg_list;
// 	f_arg_list.clear();
// 	return 0;
// }

void st_set_inloop(bool b) { in_loop = b; }

bool st_get_inloop() { return in_loop; }

void st_initialize() {
	scope = 0;
	yylineno = 1;
	scope_MAX = 0;
	nonos = 0;
	symbol_table.push_back(*(new std::vector<st_entry>));
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
	for (auto v : symbol_table) {
		std::cout << " -----------      scope #" << i++ << "      ----------- "
			<< std::endl;
		for (auto v2 : v) {
			std::cout << "\"" << v2.name << "\" [" << st_type_print[v2.type] << "] "
				<< "(line " << v2.line << ") (scope " << v2.scope << ")\n";
		}
		std::cout << std::endl;
	}
	std::cout << " ------------------------------------------- \n";
	std::cout << "\"name\"  [type]   (line)  (scope)   \n";
}
