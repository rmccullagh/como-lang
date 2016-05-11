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
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <easyio.h>
#include "ast.h"
#include "globals.h"
#include "parser.h"
#include "lexer.h"
#include "comodebug.h"

extern int yyparse(ast_node** , yyscan_t );

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

#define COMO_OP_NOP 0x00
#define COMO_OP_LOAD_CONST 0x01
#define COMO_OP_STORE_NAME 0x02
#define COMO_OP_NOT_EQUAL 0x03
#define COMO_OP_EQUAL 0x04
#define COMO_OP_LESS_THAN 0x05
#define COMO_OP_GREATER_THAN 0x06
#define COMO_OP_MINUS 0x07
#define COMO_OP_PLUS 0x08
#define COMO_OP_MUL 0x09
#define COMO_OP_DIV 0x0a
#define COMO_OP_LOAD_NAME 0x0b
#define COMO_OP_JMP_FALSE 0x0c

typedef struct como_op {
	int opcode;
	size_t jmpline;
} como_op;

#define INDENT_LOOP(i) do { \
	size_t n; \
	for(n = 0; n < i; n++) { \
		printf(" "); \
	} \
} while(0)

#define CF_STACKSIZE 1024

typedef struct como_frame {
	Object *cf_symtab;
	Object *cf_stack[CF_STACKSIZE];
	size_t cf_sp;
} como_frame;

static como_frame *cframe = NULL;

#define PUSH(e) do { \
	if(cframe->cf_sp + 1 >= CF_STACKSIZE) { \
		printf("error stack overflow tried to push onto #%zu\n", cframe->cf_sp + 1); \
		exit(1); \
	} \
	cframe->cf_stack[cframe->cf_sp++] = e; \
} while(0)

#define POP(n) do { \
	if(cframe->cf_sp - 1 == SIZE_MAX) { \
		printf("error stack underflow, tried to go before zero: %zu\n", cframe->cf_sp); \
		exit(1); \
	} \
	n = cframe->cf_stack[--cframe->cf_sp]; \
} while(0)

static int como_type_is_numeric(Object *v)
{
	return (O_TYPE(v) == IS_DOUBLE || O_TYPE(v) == IS_LONG);
}

static void como_do_binary_op_div(Object *left, Object *right)
{
	if(como_type_is_numeric(left) && como_type_is_numeric(right)) {
		if(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE) {
			double result;
			double lval;
			double rval;
			if(O_TYPE(left) == IS_DOUBLE) {
				lval = O_DVAL(left);
			}	else {
				lval = (double)(O_LVAL(left));
			}
			if(O_TYPE(right) == IS_DOUBLE) {
				rval = O_DVAL(right);
			}	else {
				rval = (double)(O_LVAL(right));
			}
			if(rval == 0) {
				como_error_noreturn("division by zero is undefined\n");
			}
			result = lval / rval;
			PUSH(newDouble(result));
		} else {
			double result;
			double lval = (double)O_LVAL(left);
			double rval = (double)O_LVAL(right);
			if(rval == 0) {
				como_error_noreturn("division by zero is undefined\n");
			}
			result = lval / rval;
			PUSH(newDouble(result));
		}
	} else {
			como_error_noreturn("Can't divide objects of non numeric type\n");
	}
}
static void como_do_binary_op_times(Object *left, Object *right)
{
	if(como_type_is_numeric(left) && como_type_is_numeric(right)) {
		if(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE) {
			double result;
			double lval;
			double rval;
			if(O_TYPE(left) == IS_DOUBLE) {
				lval = O_DVAL(left);
			}	else {
				lval = (double)(O_LVAL(left));
			}
			if(O_TYPE(right) == IS_DOUBLE) {
				rval = O_DVAL(right);
			}	else {
				rval = (double)(O_LVAL(right));
			}
			result = lval * rval;
			PUSH(newDouble(result));
		} else {
			long result;
			long lval = O_LVAL(left);
			long rval = O_LVAL(right);
			result = lval * rval;
			PUSH(newLong(result));
		}
	} else {
			como_error_noreturn("Can't multiply objects of non numeric type\n");
	}
}

static void como_do_binary_op_minus(Object *left, Object *right)
{
	if(como_type_is_numeric(left) && como_type_is_numeric(right)) {
		if(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE) {
			double result;
			double lval;
			double rval;
			if(O_TYPE(left) == IS_DOUBLE) {
				lval = O_DVAL(left);
			}	else {
				lval = (double)(O_LVAL(left));
			}
			if(O_TYPE(right) == IS_DOUBLE) {
				rval = O_DVAL(right);
			}	else {
				rval = (double)(O_LVAL(right));
			}
			result = lval - rval;
			PUSH(newDouble(result));
		} else {
			long result;
			long lval = O_LVAL(left);
			long rval = O_LVAL(right);
			result = lval - rval;
			PUSH(newLong(result));
		}
	} else {
			como_error_noreturn("Can't subtract objects of non numeric type\n");
	}
}

static void como_do_binary_op_add(Object *left, Object *right)
{
	if(como_type_is_numeric(left)) {
		if(!como_type_is_numeric(right)) {
			como_error_noreturn("Unsupported operand '+' for values\n");
		}
		if(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE) {
			double result;
			double lval;
			double rval;
			if(O_TYPE(left) == IS_DOUBLE) {
				lval = O_DVAL(left);
			}	else {
				lval = (double)(O_LVAL(left));
			}
			if(O_TYPE(right) == IS_DOUBLE) {
				rval = O_DVAL(right);
			}	else {
				rval = (double)(O_LVAL(right));
			}
			result = lval + rval;
			PUSH(newDouble(result));
		} else {
			long result;
			long lval = O_LVAL(left);
			long rval = O_LVAL(right);
			result = lval + rval;
			PUSH(newLong(result));
		}
	} else if(O_TYPE(left) == IS_STRING) {
		if(O_TYPE(right) != IS_STRING) {
			como_error_noreturn("Can't concatenate objects\n");
		}
		Object *str = stringCat(left, right);
		PUSH(str);
	} else {
			como_error_noreturn("Can't concatenate objects when both aren't type string\n");
	}
}

static inline void ast_binary_op_emit(ast_binary_op_type n)
{
	switch(n) {
		case AST_BINARY_OP_ADD: {
			fprintf(stdout, "ADD\n");
			Object *left, *right;
			POP(left);
			POP(right);
			como_do_binary_op_add(left, right);
		}	break;	
		case AST_BINARY_OP_MINUS: {
			fprintf(stdout, "SUB\n");
			Object *left, *right;
			POP(left);
			POP(right);
			como_do_binary_op_minus(left, right);
		} break;	
		case AST_BINARY_OP_TIMES: {
			fprintf(stdout, "MUL\n");
			Object *left, *right;
			POP(left);
			POP(right);
			como_do_binary_op_times(left, right);
		} break;	
		case AST_BINARY_OP_DIV: {
			fprintf(stdout, "DIV\n");
			Object *left, *right;
			POP(right);
			POP(left);
			como_do_binary_op_div(left, right);
	  }	break;	
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
		default:
			printf("not implemented %d\n", p->type);
			exit(1);
		break;
		case AST_NODE_TYPE_NUMBER:
			printf("LOAD_CONST_LONG     %ld\n", p->u1.number_value);
			PUSH(newLong(p->u1.number_value));
		break;
		case AST_NODE_TYPE_DOUBLE:
			printf("LOAD_CONST_DOUBLE     %.*G\n", DBL_DIG, p->u1.double_value);
			PUSH(newDouble(p->u1.double_value));	
		break;
		case AST_NODE_TYPE_STRING:
			printf("LOAD_CONST     %s\n", p->u1.string_value.value);
			PUSH(newString(p->u1.string_value.value));
		break;
		case AST_NODE_TYPE_ID: {
			printf("LOAD_NAME      %s\n", p->u1.id_node.name);
			Object *value;
			value = mapSearch(cframe->cf_symtab, p->u1.id_node.name);
			if(value == NULL) {
				printf("undefined variable %s\n", p->u1.id_node.name);
				exit(1);
			}
			PUSH(value);
		} break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
					ast_pretty_print(p->u1.statements_node.statement_list[i], indent);
			}		
		} break;
		case AST_NODE_TYPE_ASSIGN: {
			ast_pretty_print(p->u1.assign_node.expression, indent);
			Object *exp;
			printf("STORE_NAME     %s\n", p->u1.assign_node.name->u1.id_node.name);
			POP(exp);
			mapInsert(cframe->cf_symtab, p->u1.assign_node.name->u1.id_node.name, exp);	
			PUSH(exp);
		} break;
		case AST_NODE_TYPE_BIN_OP:
			ast_pretty_print(p->u1.binary_node.left, indent);
			ast_pretty_print(p->u1.binary_node.right, indent);
			ast_binary_op_emit(p->u1.binary_node.type);
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

	cframe = malloc(sizeof(como_frame));
	cframe->cf_sp = 0;
	int i;
	for(i = 0; i < CF_STACKSIZE; i++)
		cframe->cf_stack[i] = NULL;

	cframe->cf_symtab = newMap(2);

	ast_pretty_print(program, 0);
	Object *result;
	POP(result);
	OBJECT_DUMP(result);

	OBJECT_DUMP(cframe->cf_symtab);

	//ast_compile(argv[1], program);
	
	return 0;
}





