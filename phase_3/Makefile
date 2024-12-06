hashmap_parser: parser.cpp lex.yy.cpp kostas_sym.cpp quads.cpp actions.cpp scoping.cpp
	g++ lex.yy.cpp parser.cpp kostas_sym.cpp quads.cpp actions.cpp scoping.cpp -o $@ -g

parser: parser.cpp lex.yy.cpp symtable.cpp quads.cpp actions.cpp
	g++ lex.yy.cpp parser.cpp symtable.cpp quads.cpp actions.cpp scoping.cpp -o $@ -g

parser.cpp: parser.y	
	bison parser.y

lex.yy.cpp: scanner.l
	flex --outfile=lex.yy.cpp scanner.l

clean:
	rm -f parser hashmap_parser lex.yy.cpp parser.cpp parser.hpp
