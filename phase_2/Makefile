
parser: parser.cpp lex.yy.cpp symtable.cpp
	g++ lex.yy.cpp parser.cpp symtable.cpp -o parser -g

parser.cpp: parser.y	
	bison parser.y

lex.yy.cpp: scanner.l
	flex --outfile=lex.yy.cpp scanner.l

clean:
	rm -f parser lex.yy.cpp parser.cpp parser.hpp
