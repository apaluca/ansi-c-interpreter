ansi_c_interpreter:	ansi_c_interpreter.l ansi_c_interpreter.y ansi_c_interpreter.h ansi_c_interpreter_funcs.c
	bison -d ansi_c_interpreter.y && \
	flex -o ansi_c_interpreter.lex.c ansi_c_interpreter.l && \
	cc -g -o $@ ansi_c_interpreter.tab.c ansi_c_interpreter.lex.c ansi_c_interpreter_funcs.c -lm

clean:
	rm -f ansi_c_interpreter \
	ansi_c_interpreter.tab.c ansi_c_interpreter.tab.h ansi_c_interpreter.lex.c