#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "globals.h"

void ast_node_free(ast_node* p)
{
	if(!p)
		return;
	switch(p->type) {
		case AST_NODE_TYPE_STRING:
			free(p->u1.string_value.value);
			free(p);
		break;
		case AST_NODE_TYPE_RET:
			ast_node_free(p->u1.return_node.expr);
			free(p);
		break;
		case AST_NODE_TYPE_IF:
			ast_node_free(p->u1.if_node.condition);
			ast_node_free(p->u1.if_node.b1);
			ast_node_free(p->u1.if_node.b2);
			free(p);
		break;
		case AST_NODE_TYPE_NUMBER:
		case AST_NODE_TYPE_DOUBLE:
			free(p);
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
				ast_node_free(p->u1.statements_node.statement_list[i]);
			}
			free(p->u1.statements_node.statement_list);
			free(p);
		} 
		break;
		case AST_NODE_TYPE_BIN_OP:
			ast_node_free(p->u1.binary_node.left);
			ast_node_free(p->u1.binary_node.right);
			free(p);
		break;
		case AST_NODE_TYPE_ID:
			free(p->u1.id_node.name);
			free(p);
		break;
		case AST_NODE_TYPE_FUNC_DECL:
			free(p->u1.function_node.name);
			ast_node_free(p->u1.function_node.parameter_list);
			ast_node_free(p->u1.function_node.body);
			free(p);		
		break;
		case AST_NODE_TYPE_PRINT:
			ast_node_free(p->u1.print_node.expr);
			free(p);		
		break;
		case AST_NODE_TYPE_CALL:
			ast_node_free(p->u1.call_node.id);
			ast_node_free(p->u1.call_node.arguments);
			free(p);		
		break;
		default:
			#ifdef COMO_DEBUG
			printf("\n%s(): not implemented\n", __func__);
			#endif
		break;
	}
}
