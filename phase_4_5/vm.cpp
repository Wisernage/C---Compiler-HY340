#include "avm_libfuncs.h"
#include "avm_mem_structs.h"
#include "avm_auxiliary.h"
#include "avm_table.h"
#include "avm_execution.h"
#include "avm_structures.h"
#include "vm_elegance.h"

extern unsigned char   executionFinished;

int main (int argc, char **argv) {
	FILE  *bin_f;
	std::string fname;
	if(argc > 1) {
		fname = argv[1];
	} else {
		fname = "alpha.abc";
	}
	bin_f = fopen(fname.c_str(), "r");
	if(!bin_f) {
		std::cout << "File \033[35m" << fname << "\033[37m doesn't exist.\n";
		return 1;
	}
	decode_binary_init_vm(bin_f);
	avm_initstack();
	avm_init_libfuncs();
	while(!executionFinished) {
        execute_cycle();
    }
	
	fclose(bin_f);
}
