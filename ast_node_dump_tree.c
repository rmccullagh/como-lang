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

#include <stdio.h>
#include <float.h>
#include "ast.h"


#define CHECK_INDENT(a) do { \
	int i; \
	for(i = 0; i < a; i++) \
		printf(" "); \
} while(0)

static void ast_node_dump_tree_ex(ast_node* p, int indent);

static void ast_node_dump_number(ast_node* p)
{
	fprintf(stdout, "Num(n=%ld)", p->u1.number_value);
}

static void ast_node_dump_statement_list(ast_node* p, int indent)
{
	size_t count = p->u1.statements_node.count;
	size_t i;
	ast_node** statement_list = p->u1.statements_node.statement_list;

	printf("CompoundStatement(\n");

	for(i = 0; i < count; i++) {
		
		CHECK_INDENT(indent + 1);

		printf("[%zu]: ", i);
		ast_node_dump_tree_ex(statement_list[i], indent + 1);

		if(i + 1 != count)
			printf(",");
		
		printf("\n");
	}


	CHECK_INDENT(indent);

	printf(")");	
}

static void ast_node_dump_binary_op(ast_node* p, int indent)
{
	printf("Operation(");
	switch(p->u1.binary_node.type) {
		case AST_BINARY_OP_ADD:
			printf("+, left=");
			ast_node_dump_tree_ex(p->u1.binary_node.left, indent);
			printf(", right=");
			ast_node_dump_tree_ex(p->u1.binary_node.right, indent);
		break;
		case AST_BINARY_OP_TIMES:
			printf("*, left=");
			ast_node_dump_tree_ex(p->u1.binary_node.left, indent);
			printf(", right=");
			ast_node_dump_tree_ex(p->u1.binary_node.right, indent);
		break;
		case AST_BINARY_OP_ASSIGN:
			printf("=, left=");
			ast_node_dump_tree_ex(p->u1.binary_node.left, indent);
			printf(", right=");
			ast_node_dump_tree_ex(p->u1.binary_node.right, indent);
		break;
		default:
			printf("%s(): type not implemented\n", __func__);
		break;
	}
	printf(")");
}

static void ast_node_dump_id(ast_node* p)
{
	printf("Id(name='%s')", p->u1.id_node.name);
}

static void ast_node_dump_if(ast_node* p)
{
	printf("If(condition=");
	ast_node_dump_tree(p->u1.if_node.condition);
	printf(",b1=");
	ast_node_dump_tree(p->u1.if_node.b1);
	printf(",b2=");
	if(p->u1.if_node.b2) {
		ast_node_dump_tree(p->u1.if_node.b2);
	} else {
		printf("none");
	}
	printf(")");
}

static void ast_node_dump_func_decl(ast_node* p)
{
	printf("func %s(", p->u1.function_node.name);
	ast_node_dump_tree_ex(p->u1.function_node.parameter_list, 0);
	printf(") {\n");
	printf("}\n");	
}

static void ast_node_dump_tree_ex(ast_node* p, int indent)
{
	if(!p)
		return;
	
	switch(p->type) {
		case AST_NODE_TYPE_NUMBER:
			ast_node_dump_number(p);
		break;
		case AST_NODE_TYPE_STATEMENT_LIST:
			ast_node_dump_statement_list(p, indent);
		break;
		case AST_NODE_TYPE_BIN_OP:
			ast_node_dump_binary_op(p, indent);
		break;
		case AST_NODE_TYPE_ID:
			ast_node_dump_id(p);
		break;
		case AST_NODE_TYPE_IF:
			ast_node_dump_if(p);
		break;
		case AST_NODE_TYPE_FUNC_DECL:
			ast_node_dump_func_decl(p);
		break;
		default:
			printf("%s(): ast_node_type(%d) not implemented\n", __func__, p->type);
		break;	
	}
}

void ast_node_dump_tree(ast_node* p)
{
	ast_node_dump_tree_ex(p, 0);
	printf("\n");
}
