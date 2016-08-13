CFLAGS = -g -Wall -Wextra
LIBS = -lobject -leasyio

como: ast.o ast_node_free.o ast_node_dump_tree.o stack.o lexer.o parser.o como_compiler_ex.o como.o
	$(CC) ast.o ast_node_free.o ast_node_dump_tree.o stack.o parser.o lexer.o como_compiler_ex.o como.o -o como $(CFLAGS) $(LIBS)

ast.o: ast.c
	$(CC) $(CFLAGS) -c ast.c

como_compiler_ex.o: como_compiler_ex.c
	$(CC) $(CFLAGS) -c como_compiler_ex.c

ast_node_free.o: ast_node_free.c
	$(CC) $(CFLAGS) -c ast_node_free.c

ast_node_dump_tree.o: ast_node_dump_tree.c
	$(CC) $(CFLAGS) -c ast_node_dump_tree.c

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c

lexer.o: lexer.c
	$(CC) $(CFLAGS) -c lexer.c

lexer.c: parser.c lexer.l
	flex lexer.l

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c

parser.c: parser.y
	bison --warnings=all parser.y

como.o: como.c
	$(CC) $(CFLAGS) $(LIBS) -c como.c
clean:
	rm -f *.o lexer.c lexer.h parser.c parser.h como

