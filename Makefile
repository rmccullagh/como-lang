CFLAGS = -g -Wall -Wextra -Wno-unused-function -Wno-unused-parameter
LIBS = -lobject -leasyio

como: ast.o ast_node_dump_tree.o stack.o ast_compile.o lexer.o parser.o como.o
	$(CC) $(CFLAGS) ast.o ast_node_dump_tree.o stack.o ast_compile.o parser.o lexer.o como.o $(LIBS) -o como

ast.o: ast.c
	$(CC) $(CFLAGS) -c ast.c

ast_node_dump_tree.o: ast_node_dump_tree.c
	$(CC) $(CFLAGS) -c ast_node_dump_tree.c

ast_compile.o: ast_compile.c
	$(CC) $(CFLAGS) -c ast_compile.c

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
	$(CC) $(CFLAGS) -c como.c
clean:
	rm -f *.o lexer.c lexer.h parser.c parser.h como

