#include "actions.h"
#include "quads.h"
#include "symtable.h"
#include <assert.h>
#include <string>

extern unsigned tmp_var_count;
extern unsigned tmp_var_max;
extern std::vector<quad> quad_vec;
extern int yylineno;
extern bool hasError;
extern int yyerror(std::string message);

st_entry* newtemp() {
	st_entry *st_tmp_entry;
	std::string tmp_name;
	tmp_name = newtempname();
	st_tmp_entry = st_lookup(tmp_name, st_get_scope());
	if(st_tmp_entry)
		return st_tmp_entry;
	
	return (st_tmp_entry = st_insert(tmp_name, LOCAL_VAR));
}

std::string newtempname() {
	std::string out = "^" + std::to_string(tmp_var_count++);
	(tmp_var_count > tmp_var_max) ? (tmp_var_max = tmp_var_count) : (tmp_var_max = tmp_var_max);
	return out;
}

void resettemp() {
	tmp_var_count = 0;
}


bool istempname (std::string s) {
	return s[0] == '^';
}

bool istempexpr (expr* e) {
	return e->sym && istempname(e->sym->name);
}


std::string exp_type_to_string(expr *ex){
	
	std::string ret;
	
	switch(ex->type) {
		case BOOLEXPR_E:
			return (ex->sym->name);
		case CONSTBOOL_E:
			return (ex->value.boolConst == true? "'true'" : "'false'");
		case CONSTDOUBLE_E:
			return std::to_string(ex->value.doubleConst);
		case ARITHEXPR_E:
		case VAR_E:
			return ex->sym->name;
		case CONSTINT_E:
			return std::to_string(ex->value.intConst);
		case CONSTSTRING_E:
			return *ex->value.strConst;
		case NIL_E:
			return "NIL";
		case NEWTABLE_E:
			return "NEWTABLE_E: " + ex->sym->name;
		case PROGRAMFUNC_E:
		case LIBRARYFUNC_E:
			return "function " + ex->sym->name;
		default:
			assert(NULL);
	}

	return ret;
}

bool check_arith (expr* e, std::string context) {
	assert(e);
	if (e->type == CONSTBOOL_E ||
		e->type == CONSTSTRING_E ||
		e->type == NIL_E ||
		e->type == NEWTABLE_E ||
		e->type == PROGRAMFUNC_E ||
		e->type == LIBRARYFUNC_E ||
		e->type == BOOLEXPR_E ) {
		yyerror(context);
		hasError = true;
		return false;
	}
	return true;
}

expr * lvalue_expr (st_entry *sym) {

	assert(sym);
	expr *e = new expr();
	e->sym = sym;
	switch (sym->type) {
		case GLOBAL_VAR			:
		case FORMAL_ARG			:
		case LOCAL_VAR			: 	e->type = VAR_E; break;
		case USER_FUNC			:	e->type = PROGRAMFUNC_E; break;
		case LIB_FUNC			:	e->type = LIBRARYFUNC_E; break;
		default: assert(0);
			
	}

	return e;	
}

expr * newexpr (expr_t t) {
	expr* e = new expr();// makes all the pointers NULL
	/* memset(e, 0, sizeof(expr)); */ // Mporei na xreiastei..
	e->type = t;
	return e;
}

expr * newexpr_conststring (std::string *s) {
	expr* e = newexpr(CONSTSTRING_E);
	e->value.strConst = s;
	return e;
}

expr * newexpr_constint (int i) {
	expr* e = newexpr(CONSTINT_E);
	e->value.intConst = i;
	return e;
}

expr * newexpr_constdouble (double d) {
	expr* e = newexpr(CONSTDOUBLE_E);
	e->value.doubleConst = d;
	return e;
}

expr * newexpr_constbool (bool b) {
	expr *e = newexpr(CONSTBOOL_E);
	e->value.boolConst = b;
	return e;
}

/*
 *@brief if expr is a tableitem make quad tablegetelem else return expr
*/
expr * emit_iftableitem (expr *e) {
	expr* result;
	if (e && e->type != TABLEITEM_E)
		return e;
	else {
		result = newexpr(VAR_E);
		result->sym = newtemp();
		emit(
			TABLEGETELEM_OP,
			result,
			e,
			e->index,
			get_next_quad(),
			yylineno
			);
		return result;
	}
}

expr* emit_branch_assign_quads(expr *ex) {
	assert(ex);

	emit(ASSIGN_OP, ex, newexpr_constbool(true), NULL, get_next_quad(), yylineno);
	emit(JUMP_OP, NULL, NULL, NULL, get_next_quad() + 2, yylineno);
	emit(ASSIGN_OP, ex, newexpr_constbool(false), NULL, get_next_quad(), yylineno);

	return ex;
}

/*  */
expr* handle_bool_e(expr *arg_e)
{
    assert(arg_e);
    expr *e = arg_e;
    if(arg_e->type == BOOLEXPR_E){
        e->sym = newtemp();
		e->type = BOOLEXPR_E;
        backpatch(arg_e->truelist, get_next_quad());
        backpatch(arg_e->falselist, get_next_quad() + 2);

		emit_branch_assign_quads(e);
    }
    return e;
}



/*
 *@brief makes new member item expression
*/
expr * member_item (expr* lv, std::string *name) {

	lv = emit_iftableitem(lv);
	expr* ti = newexpr(TABLEITEM_E);
	ti->sym		= lv->sym;
	ti->index 	= newexpr_conststring(name);
	return ti;

}

expr * make_call (expr* lv, std::vector<expr*> *expr_vec) {
	expr * func = emit_iftableitem(lv);
	for(int i = expr_vec->size() - 1; i >= 0; --i ){
		emit(
			PARAM_OP,
			NULL,
			(*expr_vec)[i],
			NULL,
			0,
			yylineno
		);
	}
	
	emit(CALL_OP, func, NULL, NULL, get_next_quad(), yylineno);
	expr* result = newexpr(VAR_E);
	result->sym = newtemp();
	emit(GETRETVAL_OP, result, NULL, NULL, get_next_quad(), yylineno);
	return result;
}


expr* expr_compare_expr(expr *arg1, enum iopcode opcode, expr *arg2) {
	expr *expr_pt = newexpr(BOOLEXPR_E);
	expr_pt->sym = newtemp();
	expr_pt->truelist = new std::vector<unsigned>();
	expr_pt->falselist = new std::vector<unsigned>();
	expr_pt->truelist->push_back(get_current_quad());
	emit(opcode, NULL, arg1, arg2, get_next_quad() + 2, yylineno);
	expr_pt->falselist->push_back(get_current_quad());
	emit(JUMP_OP, NULL, NULL, NULL, get_next_quad() + 2, yylineno);
	return expr_pt;
}

expr* expr_action_expr(expr *arg1, enum iopcode opcode, expr *arg2, std::string context) {
	expr *res;
	bool is_arith = check_arith(arg1, context) && check_arith(arg2, context);
	if (is_arith) {
		res = newexpr(ARITHEXPR_E);
		res->sym = newtemp();
		emit(opcode, res, arg1, arg2, get_next_quad(), yylineno);
		return res;
	}
	return NULL;
}

void patchlabel (unsigned quadNo, unsigned label) {
	assert(quadNo < get_current_quad() && !quad_vec[quadNo].label);
	quad_vec[quadNo].label = label;
}

void backpatch(std::vector<unsigned> *vi, unsigned label){  
	for(int i = 0; i < vi->size(); ++i){
		quad_vec[(*vi)[i]].label = label;
	}
}

std::vector<unsigned>* merge(std::vector<unsigned>* list1, std::vector<unsigned>* list2){
	for(int i = 0; i < list2->size(); ++i){
		list1->push_back((*list2)[i]);
	}
	return list1;
}


/*  */
expr* true_test(expr* ex) {
	if(ex->type != BOOLEXPR_E) {
		union values val;
		unsigned char const_flag = 0;
		switch(ex->type) {
			case ARITHEXPR_E:
			case ASSIGNEXPR_E:
			case VAR_E:
				val = ex->value;
				const_flag = 1;// den einai const alla se quad while(x) == while(1);
				break;
			case CONSTBOOL_E:
				val.boolConst = ex->value.boolConst;
				const_flag = 1;
				break;
			case TABLEITEM_E:
			case NEWTABLE_E:
			case LIBRARYFUNC_E:
			case PROGRAMFUNC_E:
				val.boolConst = true;
				break;
			case CONSTINT_E:
				val.boolConst = !!ex->value.intConst;
				const_flag = 1;
				break;
			case CONSTSTRING_E:
				val.boolConst = true;
				const_flag = 1;
				break;
			case CONSTDOUBLE_E:
				if(ex->value.doubleConst == 0.0)
					val.boolConst = false;
				else
					val.boolConst = true;
				const_flag = 1;
				break;
			case NIL_E:
				val.boolConst = false;
				break;
			default:
				std::cout << "invalid expr type : " << ex->type << std::endl;
				assert(false);
		}
		expr *expr_pt;
		if(!const_flag)// for printing the desired output as quad in case of constant check
			expr_pt = new expr(BOOLEXPR_E, ex->sym, ex, val);
		else {
			expr_pt = newexpr(ex->type);// for printing correctly i need to know that type != boolexpr_e
			expr_pt->sym = newtemp();
			expr_pt->value = val;
		}
		expr_pt->truelist = new std::vector<unsigned>();
		expr_pt->falselist = new std::vector<unsigned>();
		emit(IF_EQ_OP, NULL, expr_pt, newexpr_constbool(true), get_next_quad() + 2, yylineno);
		expr_pt->truelist->push_back(get_current_quad()-1);
		emit(JUMP_OP, NULL, NULL, NULL, 0, yylineno);
		expr_pt->falselist->push_back(get_current_quad()-1);
		return expr_pt;
	}
	return ex;
}
