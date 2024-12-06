#include "quads.h"
#include "symtable.h"
#include <assert.h>
#include <vector>
#include <string>
#include <stack>
#include <iostream>
#include <fstream>

std::vector<quad> quad_vec;

std::stack<int> loop_stack;

unsigned tmp_var_count = 0;
unsigned tmp_var_max = 0;

/* unsigned loop_scope = 0; */

void emit(iopcode op, expr *result, expr *arg1, expr *arg2, unsigned label,
		unsigned line) {
	quad_vec.push_back({.op = op,
			.result = result,
			.arg1 = arg1,
			.arg2 = arg2,
			.label = label,
			.line = line});
}

expr::expr() {
	sym = NULL;
	index = NULL;
	truelist = falselist = NULL;
}

expr::expr(expr_t type, st_entry *sym, expr *index, union values value) {
	this->type = type;
	this->sym = sym;
	this->index = index;
	this->value = value;
}

call::call(std::string *name, unsigned char method, std::vector<expr*> *elist) {
	/* this->elist = new std::vector<expr*>(*elist); */ // WARNING! : Might be wrong.
	this->elist = elist;
	this->name = name;
	this->method = method;
}

static bool can_jump(iopcode op) {
	if(op == JUMP_OP || op == IF_EQ_OP || op == IF_LESS_OP || op == IF_GREATER_OP
			|| op == IF_LESS_OP || op == IF_GREATEREQ_OP || op == IF_LESSEQ_OP)
		return true;
	return false;
}

void print_line(){
	std::cout << " -------------------------------------------\n\n";
	return;
}



void print_quads(int arg) {
	std::string opcodes[] = {"ASSIGN_OP", "ADD_OP", "SUB_OP", "MUL_OP", "DIV_OP", "MOD_OP", "UMINUS_OP", "AND_OP",
	"OR_OP", "NOT_OP", "IF_EQ_OP", "IF_NOTEQ_OP", "IF_LESSEQ_OP", "IF_GREATEREQ_OP", "IF_LESS_OP", "IF_GREATER_OP",
	"CALL_OP", "PARAM_OP", "RET_OP", "GETRETVAL_OP", "FUNCSTART_OP", "FUNCEND_OP", "TABLECREATE_OP", "TABLEGETELEM_OP",
	"TABLESETELEM_OP", "JUMP_OP"};

	std::ofstream q_file;

	if(arg == 1)
    	q_file.open("quads.txt", std::ios::out);

	std::ostream & outFile = (arg ? q_file : std::cout);

	int i = 1;
    for (auto quad : quad_vec){
		outFile << i << ": " << opcodes[quad.op] << " ";
		if (quad.result != NULL) {
			outFile << quad.result->sym->name << " ";
		}
		if(quad.arg1) {
			if(quad.arg1->sym && quad.arg1->type != CONSTBOOL_E && CONSTSTRING_E
			&& quad.arg1->type != CONSTINT_E && quad.arg1->type != CONSTDOUBLE_E) {
				outFile << quad.arg1->sym->name << " ";
			}else {
				switch (quad.arg1->type) {
					case CONSTINT_E:
						outFile << quad.arg1->value.intConst << " ";
						break;
					case CONSTDOUBLE_E:
						outFile << quad.arg1->value.doubleConst << " ";
						break;
					case CONSTSTRING_E:
						outFile << *quad.arg1->value.strConst << " ";
						break;
					case CONSTBOOL_E:
					case BOOLEXPR_E:
						outFile << (quad.arg1->value.boolConst == true? "'true'" : "'false'") << " ";
						break;
					case NIL_E:
						outFile << "NIL ";
						break;
					default:
						assert(NULL);
				}
			}
		}
		if(quad.arg2) {
			if(quad.arg2->sym && quad.arg2->type != CONSTBOOL_E && CONSTSTRING_E
			&& quad.arg2->type != CONSTINT_E && quad.arg2->type != CONSTDOUBLE_E) {
				outFile << quad.arg2->sym->name << " ";
			}else {
				switch (quad.arg2->type) {
					case CONSTINT_E:
						outFile << quad.arg2->value.intConst << " ";
						break;
					case CONSTDOUBLE_E:
						outFile << quad.arg2->value.doubleConst << " ";
						break;
					case CONSTSTRING_E:
						outFile << *quad.arg2->value.strConst << " ";
						break;
					case CONSTBOOL_E:
						outFile << (quad.arg2->value.boolConst == true? "'true'" : "'false'") << " ";
						break;
					case NIL_E:
						outFile << "NIL ";
						break;
					default:
						assert(NULL);
				}
			}
		}
		if(can_jump(quad.op)) {
			if (quad.label){
				outFile << quad.label << " ";
			}
		}
		outFile << "[line " << quad.line << "]\n";
		i++;
    }
    /* std::cout << " ------------------------------------------- \n"; */
	if(arg == 1)
		q_file.close();
	
/* 	std::ifstream f("quads.txt");

    if (f.is_open())
        std::cout << f.rdbuf();

	f.close(); */
	return;
}


unsigned int get_current_quad() {
	return quad_vec.size();
}

unsigned int get_next_quad() {
	return quad_vec.size() + 1;
}

stmt_t::stmt_t(){
	make_stmt(this);
}

void make_stmt (stmt_t* s) {
	 s->breakList = s->contList = s->retList = 0;
 }

int newlist (int i) {
	quad_vec[i].label = 0;
	return i;
}

void patchlist (int list, int label) {
	while (list) {
		int next = quad_vec[list].label;
		quad_vec[list].label = label;
		list = next;
	}
}

int mergelist (int l1, int l2) {
	if (!l1)
		return l2;
	else if (!l2)
		return l1;
	else {
		int i = l1;
		while (quad_vec[i].label)
			i = quad_vec[i].label;
		quad_vec[i].label = l2;
		return l1;
	}
}
