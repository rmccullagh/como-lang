#include <stdio.h>
#include "ast.h"

int main(void)
{

	ast_node* n1 = ast_node_create_number(25);
	ast_node* n2 = ast_node_create_number(10);

	ast_node* op = ast_node_create_binary_op(AST_BINARY_OP_TIMES, n1, n2);

	ast_node_dump_tree(op);

	ast_node_free(op);
	
	return 0;
}
