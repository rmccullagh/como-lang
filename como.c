/*
 * Copyright (c) 2016 Ryan McCullagh <me@ryanmccullagh.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <easyio.h>
#include <signal.h>
#include "ast.h"
#include "globals.h"
#include "parser.h"
#include "lexer.h"

static como_global globals;

static void init_globals(void) {
	globals.filename = NULL;
	globals.filename_length = 0;
};

static void set_file_name(const char* name) {
	size_t len = strlen(name);
	globals.filename = malloc(len + 1);
	memcpy(globals.filename, name, len);
	globals.filename[len] = '\0';
	globals.filename_length = len;	
}

const char* get_file_name(void) {
	return globals.filename;
}

int yyparse(ast_node** ast, yyscan_t scanner);

ast_node* como_ast_create(const char* text)
{
	ast_node* statements;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	
	if(yylex_init(&scanner)) {
		return NULL;
	}

	state = yy_scan_string(text, scanner);

	if(yyparse(&statements, scanner)) {
		return NULL;
	}

	yy_delete_buffer(state, scanner);

	yylex_destroy(scanner);

	return statements;
}

#define INDENT_LOOP(i) do { \
	size_t n; \
	for(n = 0; n < i; n++) { \
		printf(" "); \
	} \
} while(0)

static const char * ast_binary_op_string(ast_node_binary n)
{
	switch(n.type) {
		case AST_BINARY_OP_ADD:
			return "+";
		break;	
		case AST_BINARY_OP_ASSIGN:
			return "=";
		break;	
		case AST_BINARY_OP_DOT:
			return ".";
		break;	
	}

	return "<null>";
}

static void ast_pretty_print(ast_node *p, size_t indent);

static void ast_pretty_print(ast_node *p, size_t indent)
{
	if(p == NULL) {
		fprintf(stdout, "ast_pretty_print p == NULL\n");
		return;
	}

	switch(p->type) {
		case AST_NODE_TYPE_NUMBER:
			printf("(int %ld)", p->u1.number_value);
		break;
		case AST_NODE_TYPE_DOUBLE:
			printf("(double %.*G)", DBL_DIG, p->u1.double_value);	
		break;
		case AST_NODE_TYPE_STRING:
			printf("(string '%s')", p->u1.string_value.value);
		break;
		case AST_NODE_TYPE_ID:
			printf("(id %s)", p->u1.id_node.name);
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			INDENT_LOOP(indent);
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
					ast_pretty_print(p->u1.statements_node.statement_list[i], indent);
					if(!indent) {
						printf("\n");
					}
			}		
		} break;
		case AST_NODE_TYPE_BIN_OP:
			printf("(%s ", ast_binary_op_string(p->u1.binary_node));
			ast_pretty_print(p->u1.binary_node.left, indent);
			printf(" ");
			ast_pretty_print(p->u1.binary_node.right, indent);
			printf(")");
		break;
		case AST_NODE_TYPE_CALL:
			printf("(call\n");
			indent = indent + 1;
			INDENT_LOOP(indent);
			ast_pretty_print(p->u1.call_node.expression, indent);
			printf(" ");
			ast_pretty_print(p->u1.call_node.arguments, indent);
			indent = indent > 0 ? indent- 1 : 0;
			printf(")");
		break;
		case AST_NODE_TYPE_FUNC_DEFN: {
			printf("(function %s", p->u1.function_defn_node.name->u1.id_node.name);
			indent = indent + 1;
			ast_pretty_print(p->u1.function_defn_node.parameters, indent);
			printf("\n");
			ast_pretty_print(p->u1.function_defn_node.body, indent);
			printf(")");
			indent = indent > 0 ? indent- 1 : 0;
		}
		break;
	}
}

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("usage: como [options] <file>\n");
		printf("  --print-ast    Show the Abstract Syntax Tree\n");
		printf("  --print-sym    Show the global symbol table\n");
		return 0;
	}
	int show_sym = 0;

	char* text = file_get_contents(argv[1]);

	if(!text) {
		printf("file '%s' not found\n", argv[1]);
		return 1;
	}

	init_globals();

	set_file_name(argv[1]);

	ast_node* program = como_ast_create(text);

	free(text);


	ast_pretty_print(program, 0);

	//ast_compile(argv[1], program, show_sym);

	return 0;
}





