#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

ast_node* ast_node_create_number(double value)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_NUMBER;
	retval->u1.number_value = value;
	return retval;
}

ast_node* ast_node_create_statement_list(size_t count, ...)
{
	va_list va;
	size_t i;

	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_STATEMENT_LIST;

	if(count > 0) {	
		retval->u1.statements_node.count = count;
		retval->u1.statements_node.capacity = count;
		retval->u1.statements_node.statement_list = malloc(sizeof(ast_node) * (count));

		va_start(va, count);
				
		for(i = 0; i < count; i++) {
			retval->u1.statements_node.statement_list[i] = va_arg(va, ast_node*);
		}
		
		va_end(va);
	} else {
		retval->u1.statements_node.count = 0;
		retval->u1.statements_node.capacity = 2;
		retval->u1.statements_node.statement_list = malloc(sizeof(ast_node) * 2);
		retval->u1.statements_node.statement_list[0] = NULL;
		retval->u1.statements_node.statement_list[1] = NULL;
	}

	return retval;
}

void ast_node_statement_list_push(ast_node* node, ast_node* value)
{
	if(node->type != AST_NODE_TYPE_STATEMENT_LIST) {
		printf("%s(): Node type is not of type AST_NODE_TYPE_STATEMENT_LIST\n", __func__);	 
		return;
	}


	if(node->u1.statements_node.count >= node->u1.statements_node.capacity) {	
		node->u1.statements_node.capacity += 1;
		size_t new_capacity = node->u1.statements_node.capacity;	
		node->u1.statements_node.statement_list = realloc(node->u1.statements_node.statement_list, 
									sizeof(ast_node) * new_capacity);
		node->u1.statements_node.statement_list[node->u1.statements_node.count++] = value;	
	} else {
		node->u1.statements_node.statement_list[node->u1.statements_node.count++] = value;	
	}
}

ast_node* ast_node_create_binary_op(ast_binary_op_type type, ast_node* left, ast_node* right)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_BIN_OP;
	retval->u1.binary_node.type = type;
	retval->u1.binary_node.left = left;
	retval->u1.binary_node.right = right;

	return retval;
}

void ast_node_free(ast_node* p)
{
	if(!p)
		return;
	switch(p->type) {
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
			printf("%s(): not implemented\n", __func__);
		break;
	}
}

ast_node* ast_node_create_id(const char* name)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_ID;
	size_t len = strlen(name);
	retval->u1.id_node.length = len;
	retval->u1.id_node.name = malloc(len + 1);
	memcpy(retval->u1.id_node.name, name, len + 1);
	return retval;
}

ast_node* ast_node_create_if(ast_node* condition, ast_node* b1, ast_node* b2)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_IF;
	retval->u1.if_node.condition = condition;
	retval->u1.if_node.b1 = b1;
	retval->u1.if_node.b2 = b2;
	return retval;
}

ast_node* ast_node_create_function(const char* name, ast_node* parameters, ast_node* body)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_FUNC_DECL;
	size_t len = strlen(name);
	retval->u1.function_node.name_length = len;
	retval->u1.function_node.name = malloc(len + 1);
	memcpy(retval->u1.function_node.name, name, len + 1);
	retval->u1.function_node.parameter_list = parameters;
	retval->u1.function_node.body = body;	
	return retval;
}

ast_node* ast_node_create_call(ast_node* id, ast_node* args, int lineno, int col)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_CALL;
	retval->u1.call_node.id = id;
	retval->u1.call_node.arguments = args;
	retval->u1.call_node.lineno = lineno;
	retval->u1.call_node.colno = col;
	return retval;

}

ast_node* ast_node_create_return(ast_node* expr)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_RET;
	retval->u1.return_node.expr = expr;
	return retval;
}


ast_node* ast_node_create_print(ast_node* expr)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_PRINT;
	retval->u1.print_node.expr = expr;
	return retval;
}
