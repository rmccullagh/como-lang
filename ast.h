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

#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <object.h>
#include <stdarg.h>

/*
 * http://epaperpress.com/lexandyacc/
 */
typedef enum {
	AST_NODE_TYPE_NUMBER, AST_NODE_TYPE_STRING,
	AST_NODE_TYPE_ID, AST_NODE_TYPE_DOUBLE,
	AST_NODE_TYPE_STATEMENT_LIST, AST_NODE_TYPE_BIN_OP,
	AST_NODE_TYPE_CALL,
} ast_node_type;

typedef enum {
	AST_BINARY_OP_ADD, AST_BINARY_OP_ASSIGN, AST_BINARY_OP_DOT,
} ast_binary_op_type;

typedef struct ast_node ast_node;

typedef struct {
	char* name;
	size_t length;
} ast_node_id;

typedef struct ast_node_statements ast_node_statements;

struct ast_node_statements {
	size_t	   count;
	size_t	   capacity;
	ast_node** statement_list;
};

typedef struct {
	ast_binary_op_type type;
	ast_node* left;
	ast_node* right;
} ast_node_binary;

typedef struct {
	ast_node* expression;
	ast_node* arguments;
} ast_node_call;

struct ast_node {
	ast_node_type	type;
	int lineno;
	union {
		long number_value;
		double double_value;
		struct {
			char* value;
			size_t length;
		} string_value;
		ast_node_id		      id_node;
		ast_node_statements	statements_node;
		ast_node_binary		  binary_node;
		ast_node_call       call_node;
	} u1;
};

extern ast_node* ast_node_create_number(long, int);
extern ast_node* ast_node_create_double(double, int);
extern ast_node* ast_node_create_statement_list(size_t, ...);
extern void ast_node_statement_list_push(ast_node *, ast_node *);
extern ast_node *ast_node_create_binary_op(ast_binary_op_type, 
		ast_node *, ast_node *, int);

extern ast_node* ast_node_create_id(const char *, int);
extern ast_node* ast_node_create_string_literal(const char *, int);
extern ast_node *ast_node_create_call(ast_node *, ast_node *, int);

/*
 * These functions are defined in other files outside ast.c
 */
extern void ast_node_free(ast_node* node);
extern void ast_node_dump_tree(ast_node* node);
extern void ast_compile(const char* file, ast_node* program);

#endif
