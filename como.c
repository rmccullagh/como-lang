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
#include "ast.h"
#include "globals.h"
#include "parser.h"
#include "lexer.h"

extern int yyparse(ast_node** , yyscan_t );

struct como_frame {
	Object *cf_locals;	/* variable names identified by string, and index to values */
	Object *cf_values;
	Object *cf_constants;
	Object **cf_value_stack;
	Object **cf_stacktop;
};

struct como_block {

};

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

static inline void ast_binary_op_emit(ast_binary_op_type n)
{
	switch(n) {
		case AST_BINARY_OP_ADD:
			fprintf(stdout, "ADD\n");	
		break;	
		case AST_BINARY_OP_MINUS:
			fprintf(stdout, "SUB\n");
		break;	
		case AST_BINARY_OP_TIMES:
			fprintf(stdout, "MUL\n");
		break;	
		case AST_BINARY_OP_DIV:
			fprintf(stdout, "DIV\n");
		break;	
		default :
			fprintf(stdout, "not implemented\n");
		break;
	}
}

static void ast_pretty_print(ast_node *, size_t);

static void ast_pretty_print(ast_node *p, size_t indent)
{
	if(p == NULL) {
		fprintf(stdout, "ast_pretty_print p == NULL\n");
		return;
	}

	switch(p->type) {
		case AST_NODE_TYPE_NUMBER:
			printf("LOAD_CONST     %ld\n", p->u1.number_value);
		break;
		case AST_NODE_TYPE_DOUBLE:
			printf("LOAD_CONST     %.*G\n", DBL_DIG, p->u1.double_value);	
		break;
		case AST_NODE_TYPE_STRING:
			printf("LOAD_CONST     %s\n", p->u1.string_value.value);
		break;
		case AST_NODE_TYPE_VAR:
			printf("LOAD_NAME      %s\n", p->u1.var_node.name);
		break;
		case AST_NODE_TYPE_ID:
			printf("LOAD_NAME      %s\n", p->u1.id_node.name);
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
					ast_pretty_print(p->u1.statements_node.statement_list[i], indent);
			}		
		} break;
		case AST_NODE_TYPE_ASSIGN:
			ast_pretty_print(p->u1.assign_node.expression, indent);
			printf("STORE_NAME     %s\n", p->u1.assign_node.name->u1.var_node.name);
		break;
		case AST_NODE_TYPE_BIN_OP:
			ast_pretty_print(p->u1.binary_node.left, indent);
			ast_pretty_print(p->u1.binary_node.right, indent);
			ast_binary_op_emit(p->u1.binary_node.type);
		break;
		case AST_NODE_TYPE_IF:
			printf("AST_NODE_TYPE_IF\n");
		break;
		case AST_NODE_TYPE_WHILE:
			printf("AST_NODE_TYPE_WHILE\n");
		break;
		case AST_NODE_TYPE_FUNC_DECL:
			printf("AST_NODE_TYPE_FUNC_DECL\n");
		break;
		case AST_NODE_TYPE_CALL: {
			ast_node_call call_node = p->u1.call_node;
			ast_pretty_print(call_node.id, indent);
			ast_node *args = call_node.arguments;
			ast_pretty_print(args, indent);
		}
		break;
		case AST_NODE_TYPE_RET:
			printf("AST_NODE_TYPE_RET\n");
		break;
		case AST_NODE_TYPE_PRINT:
			printf("AST_NODE_TYPE_PRINT\n");
		break;
	}
}

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("Usage: ./%s FILE\n", argv[0]);
		return 0;
	}

	char* text = file_get_contents(argv[1]);

	if(!text) {
		printf("file '%s' not found\n", argv[1]);
		return 1;
	}

	ast_node* program = como_ast_create(text);

	free(text);

	ast_pretty_print(program, 0);

	//ast_compile(argv[1], program);
	//printf("%d\n", program->type);

	return 0;
}





