hashmap_parser: parser.cpp lex.yy.cpp kostas_sym.cpp quads.cpp actions.cpp scoping.cpp avm_structures.cpp #avm_table.cpp
	g++ lex.yy.cpp parser.cpp kostas_sym.cpp quads.cpp actions.cpp scoping.cpp avm_structures.cpp -o $@ -g

vm: vm.cpp vm_elegance.cpp avm_mem_structs.cpp avm_libfuncs.cpp avm_auxiliary.cpp avm_table.cpp avm_execution.cpp
	g++ vm.cpp vm_elegance.cpp avm_mem_structs.cpp avm_libfuncs.cpp avm_auxiliary.cpp avm_table.cpp avm_execution.cpp -o alpha_vm -g

parser: parser.cpp lex.yy.cpp symtable.cpp quads.cpp actions.cpp 
	g++ lex.yy.cpp parser.cpp symtable.cpp quads.cpp actions.cpp scoping.cpp -o $@ -g

parser.cpp: parser.y	
	bison parser.y

lex.yy.cpp: scanner.l
	flex --outfile=lex.yy.cpp scanner.l

clean:
	rm -f parser hashmap_parser alpha_vm lex.yy.cpp parser.cpp parser.hpp alpha.abc alpha.abc4debug