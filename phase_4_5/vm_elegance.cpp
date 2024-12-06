#include "vm_elegance.h"
#include <vector>
#include "avm_structures.h"
#include "scoping.h"
#include "avm_mem_structs.h"

extern avm_memcell stack[AVM_STACKSIZE];

extern unsigned        codeSize;

std::vector<double>         numConsts;
std::vector<std::string> 	stringConsts;
std::vector<std::string> 	namedLibFuncs;// namedLibFuncs[result->val] <- eksagwgh onomatos libF // result <- type LIBFUNC_A
std::vector<userfunc> 		userFuncs;
unsigned totalProgVars;

instruction*    code = (instruction*) 0;

void tmp_print_instructions ();

void decode_binary_init_vm(FILE *bin_f) {
    unsigned magic, len;
    fread(&magic, sizeof(unsigned), 1, bin_f);
	if(magic != MAGIC_NUM) {
		// std::cout << "\033[31mERROR\033[37m: This binary file is not produced by alpha lang!\n";
		exit(1);
	}
	// std::cout << magic << std::endl;
	
	fread(&totalProgVars, sizeof(unsigned), 1, bin_f);// progVar len
	// std::cout << totalProgVars << std::endl;

	for(int i = 0; i < totalProgVars; ++i) {// ProgVars
		avm_memcell prog_var_memcell;

		fread(&prog_var_memcell, sizeof(avm_memcell), 1, bin_f);
		prog_var_memcell.type = UNDEF_M;
		stack[AVM_STACKSIZE - 1 - i] = prog_var_memcell;
	}
	
	fread(&len, sizeof(unsigned), 1, bin_f);// constStr len
	// std::cout << len << std::endl;
	for(int i = 0; i < len; ++i) {// strings
		unsigned str_size;
		fread(&str_size, sizeof(unsigned), 1, bin_f);
		std::string *tmp_str = new std::string;
		tmp_str->resize(str_size);
		for (int j = 0; j < str_size; ++j) {
			char c;
			fread(&c, 1, 1, bin_f);
			tmp_str->at(j) = c;
		}
		stringConsts.push_back(*tmp_str);
		delete tmp_str;
		// std::cout << str_size << "," << stringConsts.back() << " ";
	}
	fread(&len, sizeof(unsigned), 1, bin_f);// constNum len
	// std::cout << std::endl << len << std::endl;
	for(int i = 0; i < len; ++i) {
		double n;
		fread(&n, sizeof(double), 1, bin_f);
		numConsts.push_back(n);
		// std::cout << numConsts.back() << " ";
	}
	fread(&len, sizeof(unsigned), 1, bin_f);// libFuncs len
	// std::cout << std::endl << len << std::endl;
	for(int i = 0; i < len; ++i) {
		unsigned name_len;
		fread(&name_len, sizeof(unsigned), 1, bin_f);
		char buffer[64];// libf names are known and are short
		fread(&buffer, name_len, 1, bin_f);
		buffer[name_len] = 0;
		namedLibFuncs.push_back(std::string(buffer));
		// std::cout << namedLibFuncs.back() << " ";
	}
	fread(&len, sizeof(unsigned), 1, bin_f);// UserFuncs len
	// std::cout << std::endl << len << std::endl;
	for(int i = 0; i < len; ++i) {
		unsigned name_len, address, local_size, arg_size;
		fread(&name_len, sizeof(unsigned), 1, bin_f);
		std::string *tmp_str = new std::string;
		tmp_str->resize(name_len);
		for(int j = 0; j < name_len; ++j) {
			char c;
			fread(&c, 1, 1, bin_f);
			tmp_str->at(j) = c;
		}
		fread(&address, sizeof(unsigned), 1, bin_f);
		fread(&local_size, sizeof(unsigned), 1, bin_f);
		fread(&arg_size, sizeof(unsigned), 1, bin_f);
		userFuncs.push_back( userfunc{ address,
			local_size,
			arg_size,
			tmp_str});
	std::cout << *(userFuncs[i].id) << ","
	<< userFuncs[i].address << "," << userFuncs[i].localSize << "," << userFuncs[i].argSize << " ";
	}
	// std::cout << std::endl;

	// instructions opcode [vmarg] [vmarg] [vmarg] [srcLine]
    fread(&codeSize, sizeof(unsigned long), 1, bin_f);
	code = (instruction*) malloc (codeSize*sizeof(instruction));
	// std::cout << codeSize << std::endl;
    for(int i = 0; i < codeSize; ++i) {
        enum vmopcode op;
        fread(&op, sizeof(enum vmopcode), 1, bin_f);
        vmarg tmp_arg;
        instruction tmp_instr;
        unsigned srcLine;
        tmp_instr.opcode = op;
        switch(op) {
            case ASSIGN_V:// 4 operands inc srcL
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.result =  tmp_arg;       
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.arg1 =  tmp_arg;
				tmp_instr.arg2.val = (unsigned)-1;
				tmp_instr.arg2.type = UNDEF_A;
				fread(&srcLine, sizeof(unsigned), 1, bin_f);
                tmp_instr.srcLine =  srcLine;
                code[i] = tmp_instr;
                break;
            case ADD_V:// 5 operands including srcLine
            case SUB_V:
            case MUL_V:
            case DIV_V:
            case MOD_V:
            case JEQ_V:
            case JNE_V:
            case JLT_V:
            case JGT_V:
            case JLE_V:
            case JGE_V:
            case TABLEGETELEM_V:
            case TABLESETELEM_V:
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.result =  tmp_arg;
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.arg1 =  tmp_arg;
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.arg2 =  tmp_arg;
                fread(&srcLine, sizeof(unsigned), 1, bin_f);
                tmp_instr.srcLine =  srcLine;
                code[i] = tmp_instr;
                break;
            case FUNCENTER_V:
            case FUNCEXIT_V:
            case JUMP_V:
            case NEWTABLE_V:
            case CALL_V:
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.result =  tmp_arg;
                tmp_instr.arg1.val = (unsigned)-1;
				tmp_instr.arg1.type = UNDEF_A;
                tmp_instr.arg2.val = (unsigned)-1;
				tmp_instr.arg2.type = UNDEF_A;
                fread(&srcLine, sizeof(unsigned), 1, bin_f);
                tmp_instr.srcLine =  srcLine;
                code[i] = tmp_instr;
                break;
            case PUSHARG_V:
                tmp_instr.result.val = (unsigned)-1;
				tmp_instr.result.type = UNDEF_A;
                fread(&tmp_arg, sizeof(vmarg), 1, bin_f);
                tmp_instr.arg1 = tmp_arg;
                tmp_instr.arg2.val = (unsigned)-1;
				tmp_instr.arg2.type = UNDEF_A;
                fread(&srcLine, sizeof(unsigned), 1, bin_f);
                tmp_instr.srcLine =  srcLine;
                code[i] = tmp_instr;
                break;
            default:
                assert(0);
        }
    }
    // tmp_print_instructions();
}


void tmp_print_instructions () {// for debug
	std::string instrCodes[] = {"ASSIGN_V", "ADD_V", "SUB_V", "MUL_V", "DIV_V", "MOD_V", "UMINUS_V", "AND_V",
	"OR_V", "NOT_V", "JEQ_V", "JNE_V", "JLE_V", "JGE_V", "JLT_V", "JGT_V",
	"CALL_V", "PUSHARG_V", "FUNCENTER_V", "FUNCEXIT_V", "NEWTABLE_V", "TABLEGETELEM_V",
	"TABLESETELEM_V", "JUMP_V", "NOP_V"};
	std::string argCodes[] = {"LABEL_A", "GLOBAL_A", "FORMAL_A", "LOCAL_A", "NUMBER_A", "STRING_A", "BOOL_A",
	"NIL_A", "USERFUNC_A", "LIBFUNC_A", "RETVAL_A"};

	for (int i = 0; i < codeSize; ++i) {
		std::cout << i << ": " << instrCodes[code[i].opcode] << " ";
		if(code[i].result.val != (unsigned)-1)
			std::cout << argCodes[code[i].result.type] << (code[i].result.type == LABEL_A?
						"->" + std::to_string(code[i].result.val) + " ":
						(code[i].result.type == USERFUNC_A?"->"+
						 std::to_string(userFuncs[code[i].result.val].address) + " ":" "));
		else
			std::cout << "unused_result" << " ";
		if(code[i].arg1.val != (unsigned)-1)
			std::cout << argCodes[code[i].arg1.type] << " ";
		else
			std::cout << "unused_arg1" << " ";
		if(code[i].arg2.val != (unsigned)-1)
			std::cout << argCodes[code[i].arg2.type] << " ";
		else
			std::cout << "unused_arg2" << " ";
		std::cout << code[i].srcLine << std::endl;
	}
	std::cout << " -----------------------------------------------\n";
}
