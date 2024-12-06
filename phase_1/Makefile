
scanner: lex.yy.c
	g++ lex.yy.c -o scanner

lex.yy.c: scanner.l
	flex scanner.l

clean:
	rm -f scanner scanner.h lex.yy.c

stringTests: stringPassTest stringErrorTest
stringErrorTest: scanner
	echo "\n---Running the error string tests---\n" && \
	./scanner tests_ph1/string_err1.alpha && \
		./scanner tests_ph1/string_err2.alpha
stringPassTest: scanner
		echo "\n\n---Running the pass string test---\n" \
		&& ./scanner tests_ph1/string.alpha
