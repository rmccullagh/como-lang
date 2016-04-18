/*
*  Copyright (c) 2016 Ryan McCullagh <me@ryanmccullagh.com>
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "globals.h"

ast_node* ast_node_create_number(long value, int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_NUMBER;
	retval->lineno = lineno;
	retval->colno = colno;
	retval->u1.number_value = value;
	return retval;
}

ast_node* ast_node_create_double(double value, int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_DOUBLE;
	retval->lineno = lineno;
	retval->colno = colno;
	retval->u1.double_value = value;
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

ast_node* ast_node_create_binary_op(ast_binary_op_type type, ast_node* left, ast_node* right, 
		int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_BIN_OP;
	retval->colno = colno;
	retval->lineno = lineno;
	retval->u1.binary_node.type = type;
	retval->u1.binary_node.left = left;
	retval->u1.binary_node.right = right;

	return retval;
}

ast_node* ast_node_create_id(const char* name, int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_ID;
	retval->colno = colno;
	retval->lineno = lineno;
	size_t len = strlen(name);
	retval->u1.id_node.length = len;
	retval->u1.id_node.name = malloc(len + 1);
	memcpy(retval->u1.id_node.name, name, len + 1);
	return retval;
}

ast_node* ast_node_create_string_literal(const char* str, int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_STRING;
	retval->lineno = lineno;
	retval->colno = colno;
	retval->u1.string_value.length = strlen(str);
	retval->u1.string_value.value = malloc(retval->u1.string_value.length + 1);
	memcpy(retval->u1.string_value.value, str, retval->u1.string_value.length);
	retval->u1.string_value.value[retval->u1.string_value.length] = '\0';
	return retval;
}

ast_node *ast_node_create_call(ast_node *expression, ast_node *args, int lineno, int colno)
{
	ast_node* retval = malloc(sizeof(ast_node));
	retval->type = AST_NODE_TYPE_CALL;
	retval->lineno = lineno;
	retval->colno = colno;
	retval->u1.call_node.expression = expression;
	retval->u1.call_node.arguments = args;
	return retval;
}


