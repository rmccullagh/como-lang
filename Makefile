CFLAGS = -g -Wall -Wextra
LIBS = -lobject -leasyio

como: ast_compile.o ast.o object_api.o ast_node_free.o stack.o lexer.o parser.o como.o
	$(CC) $(CFLAGS) ast_compile.o ast.o object_api.o ast_node_free.o stack.o parser.o lexer.o como.o $(LIBS) -o como

ast_compile.o: ast_compile.c
	$(CC) $(CFLAGS) -c ast_compile.c

ast.o: ast.c
	$(CC) $(CFLAGS) -c ast.c

ast_node_free.o: ast_node_free.c
	$(CC) $(CFLAGS) -c ast_node_free.c

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c

object_api.o: object_api.c
	$(CC) $(CFLAGS) -c object_api.c

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

