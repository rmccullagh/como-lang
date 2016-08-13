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
#include <string.h>
#include <easyio.h>
#include <object.h>
#include <assert.h>

#include "ast.h"
#include "como_execute.h"
#include "stack.h"
#include "como_compiler.h"
#include "comodebug.h"
#include "como_opcode.h"
#include "globals.h"
#include "parser.h"
#include "lexer.h"

#define COMO_IS_FUNCTION (1 << 0)

como_executor_globals *ceg;

int yyparse(ast_node** ast, yyscan_t scanner);


static void debug_code_to_output(FILE *fp) {
		
		fprintf(fp, "*** BEGIN CODE ***\n");
		size_t i;
		for(i = 0; i < CG(op_array)->size; i++) {
			como_op_code *c = CG(op_array)->table[i];
			switch(c->op_code) {
				case IREM: {
					fprintf(fp, "\tIREM\n");
					break;
				}
				case IRETURN: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tIRETURN %s\n", value);
					free(value);
					break;
				}
				case CALL_FUNCTION: {
					fprintf(fp, "\tCALL_FUNCTION\n");
					break;
				}
				case DEFINE_FUNCTION: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tDEFINE_FUNCTION %s\n", value);
					free(value);
					break;
				}
				case LOAD_CONST: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tLOAD_CONST %s\n", value);
					free(value);
					break;
				}
				case STORE_NAME: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tSTORE_NAME %s\n", value);
					free(value);
					break;
				}
				case LOAD_NAME: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tLOAD_NAME %s\n", value);
					free(value);
					break;
				}
				case IS_LESS_THAN: {
					fprintf(fp, "\tIS_LESS_THAN\n");
					break;
				}
				case JZ: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tJZ %s\n", value);
					free(value);
					break;
				}
				case IPRINT: {
					fprintf(fp,"\tPRINT\n");
					break;
				}
				case IADD: {
					fprintf(fp, "\tADD\n");
					break;
				}
				case IMINUS: {
					fprintf(fp, "\tSUB\n");
					break;
				}
				case IDIV: {
					fprintf(fp, "\tDIV\n");
					break;
				}
				case ITIMES: {
					fprintf(fp, "\tTIMES\n");
					break;
				}
				case JMP: {
					char *value = objectToString(c->operand);
					fprintf(fp, "\tJMP %s\n", value);
					free(value);
					break;
				}
				case NOP: {
					fprintf(fp,"\tNOP\n");
					break;
				}
				case LABEL: {
					char *value = objectToString(c->operand);
					fprintf(fp, "LABEL %s\n", value);
					free(value);
					break;
				}
				case HALT: {
					fprintf(fp, "\tHALT\n");	
					break;
				}
				case IS_EQUAL: {
					fprintf(fp, "\tIS_EQUAL\n");
					break;
				}
				case IS_GREATER_THAN: {
					fprintf(fp, "\tIS_GREATER_THAN\n");
					break;
				}
				case IS_NOT_EQUAL: {
					fprintf(fp, "\tIS_NOT_EQUAL\n");
					break;
				}
				case IS_GREATER_THAN_OR_EQUAL: {
					fprintf(fp, "\tIS_GREATER_THAN_OR_EQUAL\n");
					break;
				}
				case IS_LESS_THAN_OR_EQUAL: {
					fprintf(fp, "\tIS_LESS_THAN_OR_EQUAL\n");
					break;
				}
				case POSTFIX_INC: {
					fprintf(fp, "\tPOSTFIX_INC\n");
					break;
				}
				case POSTFIX_DEC: {
					fprintf(fp, "\tPOSTFIX_DEC\n");
					break;
				}
				case UNARY_MINUS: {
					fprintf(fp, "\tUNARY_MINUS\n");
					break;
				}
			}
		}
		fprintf(fp, "*** END CODE ***\n");
}

static int como_compile(ast_node *);
static void como_vm(como_op_array *);

#define EMPTY() (CG(op_array)->frame->cf_sp == 0)

#define PUSH(e) do { \
		if(CG(op_array)->frame->cf_sp >= COMO_DEFAULT_FRAME_STACKSIZE) { \
			como_error_noreturn("error stack overflow tried to push onto #%zu", CG(op_array)->frame->cf_sp); \
			exit(1); \
		} else { \
			CG(op_array)->frame->cf_stack[CG(op_array)->frame->cf_sp++] = e; \
			CG(op_array)->frame->cf_stack_size++; \
		} \
} while(0)

#define POP(n) do { \
		if(CG(op_array)->frame->cf_sp == 0) { \
			como_error_noreturn("stack underflow, tried to go before 0"); \
		} else { \
			n = CG(op_array)->frame->cf_stack[--CG(op_array)->frame->cf_sp]; \
			--CG(op_array)->frame->cf_stack_size; \
		} \
} while(0)

#define BINARY_OP_SETUP \
		Object *left; \
		Object *right; \
		POP(right); \
		POP(left); \

#define ENSURE_NUMERIC_OPERANDS \
	if(O_TYPE(left) != IS_LONG && O_TYPE(right) != IS_LONG) { \
		fprintf(stderr, "como: can't invoke binary operation on non numeric arguments\n"); \
		exit(1); \
	} \

#define TARGET(x) \
	case x: \

#define VM_CONTINUE \
	CG(op_array)->pc++; \
	break; \

#define OPERANDS_ARE_BOTH_NUMERIC \
	(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG)

#define BINARY_OP_FREE
#define FREE_OP(x)

#define ENSURE_POINTER_TYPE(o) do { \
	if(O_TYPE((o)) != IS_POINTER) { \
		como_error_noreturn("argument must be an internal IS_POINTER type, got (%d): %s", O_TYPE((o)), \
			objectTypeStr((o))); \
	} \
} while(0)

static void como_vm(como_op_array *oparray) {
	while(1) {
		size_t pc = oparray->pc;
		como_op_code *c = oparray->table[pc];
		assert(c);
		
		switch(c->op_code) {
			TARGET(POSTFIX_INC) {
				Object *name = NULL;
				if(oparray->frame->cf_current_symtab == NULL) {
					name = mapSearchEx(oparray->frame->cf_symtab, 
						O_SVAL(c->operand)->value);
				} else {
					name = mapSearchEx(oparray->frame->cf_current_symtab, 
						O_SVAL(c->operand)->value);
				}

				if(name == NULL) {
					fprintf(stderr, "Undefined variable %s\n", O_SVAL(c->operand)->value);
					PUSH(newLong(0));	
				} else {
					if(O_TYPE(name) != IS_LONG) {
						como_error_noreturn("unsupported value for POSTFIX_INC");
					} else {
						long oldvalue = O_LVAL(name);
						O_LVAL(name) = oldvalue + 1;
						PUSH(newLong(oldvalue));
					}
				}
				VM_CONTINUE
			}	
			TARGET(POSTFIX_DEC) {
				Object *name = NULL;
				if(oparray->frame->cf_current_symtab == NULL) {
					name = mapSearchEx(oparray->frame->cf_symtab, 
						O_SVAL(c->operand)->value);
				} else {
					name = mapSearchEx(oparray->frame->cf_current_symtab, 
						O_SVAL(c->operand)->value);
				}
				if(name == NULL) {
					fprintf(stderr, "undefined variable '%s' for POSTFIX_DEC\n", 
							O_SVAL(c->operand)->value);
					PUSH(newLong(0));	
				} else {
					if(O_TYPE(name) != IS_LONG) {
						como_error_noreturn("unsupported value for POSTFIX_DEC");
					} else {
						long oldvalue = O_LVAL(name);
						O_LVAL(name) = oldvalue - 1;
						PUSH(newLong(oldvalue));
					}
				}
				VM_CONTINUE
			}	
			TARGET(UNARY_MINUS) {
				Object *value;
				POP(value);
				if(O_TYPE(value) == IS_LONG) {
					PUSH(newLong(-O_LVAL(value)));
				} else {
					como_error_noreturn("unsupported value for UNARY_MINUS");
				}
				VM_CONTINUE
			}
			TARGET(IRETURN) {
				if(O_LVAL(c->operand) == 0L) {
					como_debug("returning from function without return value");
					oparray->frame->cf_retval = newLong(0L);
				} else {
					POP(oparray->frame->cf_retval);
				}
				return;
			}
			TARGET(DEFINE_FUNCTION) {
				PUSH(c->operand);
				VM_CONTINUE
			}
			TARGET(LOAD_CONST) {
				PUSH(c->operand);
				VM_CONTINUE
			}
			TARGET(STORE_NAME) {
				Object *v;
				POP(v);
				v = copyObject(v);
				/* make sure to copy the value of v 
				   because v might be coming from a variable
				 */
				if(oparray->frame->cf_current_symtab == NULL) {
					mapInsertEx(oparray->frame->cf_symtab, 
						O_SVAL(c->operand)->value, v);
				} else {
					mapInsertEx(oparray->frame->cf_current_symtab, 
						O_SVAL(c->operand)->value, v);
				}
				PUSH(v);
				VM_CONTINUE
			}
			TARGET(LOAD_NAME) {
				Object *value = NULL;
				if(oparray->frame->cf_current_symtab == NULL) {
					value = mapSearchEx(oparray->frame->cf_symtab, 
						O_SVAL(c->operand)->value);
				} else {
					value = mapSearchEx(oparray->frame->cf_current_symtab, 
						O_SVAL(c->operand)->value);
				}

				if(value == NULL) {
					fprintf(stderr, "Undefined variable %s\n", O_SVAL(c->operand)->value);
					PUSH(newLong(0L));
				} else {
					PUSH(copyObject(value));
				}
				VM_CONTINUE
			}
			TARGET(IS_LESS_THAN) {
				BINARY_OP_SETUP
				PUSH(newLong(objectValueIsLessThan(left, right)));
				VM_CONTINUE
			}
			TARGET(JZ) {
				Object *cond;
				POP(cond);
				if(O_TYPE(cond) == IS_LONG && O_LVAL(cond) == 0) {
					CG(op_array)->pc = O_LVAL(c->operand);		
					break;			
				}
			 	FREE_OP(cond);	
				VM_CONTINUE
			}
			TARGET(IPRINT) {
				Object *value;
				POP(value);
				size_t len = 0;
				char *sval = objectToStringLength(value, &len);
				fprintf(stdout, "%s\n", sval);
				fflush(stdout);
				free(sval);
				FREE_OP(value);
				VM_CONTINUE
			}
			TARGET(IADD) {
				BINARY_OP_SETUP
				if(OPERANDS_ARE_BOTH_NUMERIC) {
					PUSH(newLong(O_LVAL(left) + O_LVAL(right)));
				} else {
					//Object *result;
					//result = object_concat_helper(left, right);
					char *sval = objectToString(left);
					char *rval = objectToString(right);
					Object *s1 = newString(sval);
					Object *s2 = newString(rval);
					Object *s3 = stringCat(s1, s2);
					free(sval);
					free(rval);
					objectDestroy(s1);
					objectDestroy(s2);
					PUSH(s3);	
				}
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(IDIV) {
				BINARY_OP_SETUP
				ENSURE_NUMERIC_OPERANDS
				if(O_LVAL(right) == 0) {
					fprintf(stderr, "como: can't divide by zero\n");
					exit(1);
				}
				PUSH(newLong(O_LVAL(left) / O_LVAL(right)));
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(IMINUS) {
				BINARY_OP_SETUP
				ENSURE_NUMERIC_OPERANDS
				PUSH(newLong(O_LVAL(left) - O_LVAL(right)));
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(ITIMES) {
				BINARY_OP_SETUP
				ENSURE_NUMERIC_OPERANDS
				PUSH(newLong(O_LVAL(left) * O_LVAL(right)));
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(IREM) {
				BINARY_OP_SETUP
				ENSURE_NUMERIC_OPERANDS
				PUSH(newLong(O_LVAL(left) % O_LVAL(right)));
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(JMP) {
				CG(op_array)->pc = O_LVAL(c->operand);
				break;
			}
			TARGET(NOP) {
				VM_CONTINUE
			}
			TARGET(LABEL) {
				VM_CONTINUE
			}
			TARGET(HALT) {
				return;
			}
			TARGET(IS_EQUAL) {
				BINARY_OP_SETUP
				PUSH(newLong(objectValueCompare(left, right)));
				BINARY_OP_FREE
				VM_CONTINUE
			}
			TARGET(IS_GREATER_THAN) {
				BINARY_OP_SETUP
				PUSH(newLong(
							(long)objectValueIsGreaterThan(left, right)
					)
				);
				VM_CONTINUE
			}
			TARGET(IS_NOT_EQUAL) {
				BINARY_OP_SETUP
				PUSH(newLong(
					(long)(!objectValueCompare(left, right))
					)
				);
				VM_CONTINUE
			}
			TARGET(IS_GREATER_THAN_OR_EQUAL) {
				BINARY_OP_SETUP
				PUSH(newLong(
					(long)
						(objectValueCompare(left, right) ||
						 	objectValueIsGreaterThan(left,right)
						)
					)
				);
				VM_CONTINUE		
			}
			TARGET(IS_LESS_THAN_OR_EQUAL) {
				BINARY_OP_SETUP
				if(objectValueCompare(left, right) || objectValueIsLessThan(left, right)) {
					PUSH(newLong(1L));
				} else {
					PUSH(newLong(0L));
				}
				VM_CONTINUE		
			}
			TARGET(CALL_FUNCTION) {
				como_debug("calling function `%s'", O_SVAL(c->operand)->value);
				Object *fn;
				Object *arglen;
				Object *scope = newMap(4);
				
				Object* old_current_symbol_table = oparray->frame->cf_current_symtab;

				como_function *function;
				POP(fn);
				if(O_TYPE(fn) != IS_POINTER) {
					fprintf(stderr, "TypeError: name '%s' is not callable\n",
						O_SVAL(c->operand)->value);
						VM_CONTINUE
				}
				function = (como_function *)O_PTVAL(fn);
				POP(arglen);
				como_debug("calling with %ld args", O_LVAL(arglen));
				if(O_AVAL(function->arguments)->size != (size_t)O_LVAL(arglen)) {
					fprintf(stderr, "como: error: function expects %zu arguments, %zu given\n",
						O_AVAL(function->arguments)->size, (size_t)O_LVAL(arglen));
					VM_CONTINUE
				}
				long i = O_LVAL(arglen);
				while(i--) {
					Object *arg_value;
					POP(arg_value);
					Object *arg_name = O_AVAL(function->arguments)->table[i];
					/* It's important to copy the value of arg_value (mapInsert) here
					   as, if we don't, then arguments will actually be passed by reference, not value
					 */
					como_debug("populating symbol table");
					mapInsert(scope, O_SVAL(arg_name)->value, arg_value);

				}
				
				oparray->frame->cf_current_symtab = scope;

				(void)como_vm(function->op_array);

				Object *retval = oparray->frame->cf_retval;
				PUSH(retval);

				oparray->frame->cf_current_symtab = oparray->frame->cf_symtab;

				VM_CONTINUE
			}
		}
	}
}

static int como_compile(ast_node* p)
{
	assert(p);

	switch(p->type) {
		default:
			printf("%s(): invalid node type(%d)\n", __func__, p->type);
			exit(1);
		break;
		case AST_NODE_TYPE_STRING:
			como_emit(CG(op_array), LOAD_CONST, newString(p->u1.string_value.value));
		break;
		case AST_NODE_TYPE_PRINT:
			como_compile(p->u1.print_node.expr);
			como_emit(CG(op_array), IPRINT, NULL);
		break;
		case AST_NODE_TYPE_NUMBER:
			como_emit(CG(op_array), LOAD_CONST, newLong(p->u1.number_value));
		break;
		case AST_NODE_TYPE_ID:
			como_emit(CG(op_array), LOAD_NAME, newString(p->u1.id_node.name));
		break;
		case AST_NODE_TYPE_RET:
			if(p->u1.return_node.expr != NULL) {
				como_compile(p->u1.return_node.expr);
				como_emit(CG(op_array), IRETURN, newLong(1L));
			} else {
				como_emit(CG(op_array), IRETURN, newLong(0L));
			}
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
				ast_node* stmt = p->u1.statements_node.statement_list[i];
				como_compile(stmt);
			}
		} 
		break;
		case AST_NODE_TYPE_WHILE: {
			Object *l = newLong((long)CG(op_array)->size);
			como_emit(CG(op_array),LABEL, l);
			
			Object *l2 = newLong(0);
			como_compile(p->u1.while_node.condition);
			como_emit(CG(op_array),JZ, l2);
			como_compile(p->u1.while_node.body);
			como_emit(CG(op_array), JMP, newLong(O_LVAL(l)));
    	
			Object *l3 = newLong((long)CG(op_array)->size);
    
			como_emit(CG(op_array), LABEL, l3);
    	
			O_LVAL(l2) = O_LVAL(l3);
		
		}
		break;
		case AST_NODE_TYPE_FOR: {
			Object *l = newLong((long)CG(op_array)->size);
			como_emit(CG(op_array), LABEL, l);
			
			Object *l2 = newLong(0);
			como_compile(p->u1.for_node.initialization);
			como_compile(p->u1.for_node.condition);
			como_emit(CG(op_array), JZ, l2);

			Object *l4 = newLong((long)CG(op_array)->size);
			/* label for the body */
			como_emit(CG(op_array), LABEL, l4);
			como_compile(p->u1.for_node.body);
			
			como_compile(p->u1.for_node.final_expression);

			como_compile(p->u1.for_node.condition);
			como_emit(CG(op_array), JZ, l2);

			como_emit(CG(op_array), JMP, newLong(O_LVAL(l4)));
    	
			Object *l3 = newLong((long)CG(op_array)->size);
    
			como_emit(CG(op_array), LABEL, l3);
    	
			O_LVAL(l2) = O_LVAL(l3);
		}
		break;
		case AST_NODE_TYPE_IF: {
			Object *l2 = newLong(0);
			Object *l4 = newLong(0);
			como_compile(p->u1.if_node.condition);
		
			como_emit(CG(op_array),JZ, l2);

			como_compile(p->u1.if_node.b1);

			como_emit(CG(op_array),JMP, l4);

			
			Object *l3 = newLong((long)CG(op_array)->size);
    	
    		como_emit(CG(op_array),LABEL, l3);

			if(p->u1.if_node.b2 != NULL) {
				como_compile(p->u1.if_node.b2);
			}
    	
			O_LVAL(l2) = O_LVAL(l3);
			O_LVAL(l4) = (long)CG(op_array)->size;

			como_emit(CG(op_array),LABEL, newLong((long)CG(op_array)->size));

		} 
		break;
		case AST_NODE_TYPE_FUNC_DECL: {	
			como_function *function = malloc(sizeof(como_function));
			function->name = newString(p->u1.function_node.name);
			size_t parameter_count = 
				p->u1.function_node.parameter_list->u1.statements_node.count;
			function->arguments = newArray(parameter_count);
			function->op_array = como_op_array_create();
			size_t i;
			for(i = 0; i < parameter_count; i++) {
				arrayPush(function->arguments,
					newString(p->u1
								.function_node
								.parameter_list->u1
								.statements_node
								.statement_list[i]->u1
								.id_node
								.name));
			}
			como_op_array *old_op_array = CG(op_array);

			CG(op_array) = function->op_array;

			como_compile(p->u1.function_node.body);

			if(CG(op_array)->table[CG(op_array)->size - 1]->op_code != IRETURN) {
				como_debug("automatically inserting IRETURN for function %s", p->u1.function_node.name);
				como_emit(CG(op_array), LOAD_CONST, newLong(0L));
  				como_emit(CG(op_array), IRETURN, newLong(1L));				
			}

			CG(op_array) = old_op_array;

			como_emit(CG(op_array), DEFINE_FUNCTION, newPointer((void *)function));
			como_emit(CG(op_array), STORE_NAME, newString(p->u1.function_node.name));	

			break;
		} 
		case AST_NODE_TYPE_CALL: {
			como_compile(p->u1.call_node.arguments);
			como_emit(CG(op_array), LOAD_CONST, newLong((long)p->u1.call_node.arguments->u1.statements_node.count));
			como_emit(CG(op_array), LOAD_NAME, newString(p->u1.call_node.id->u1.id_node.name));
			como_emit(CG(op_array), CALL_FUNCTION, newString(p->u1.call_node.id->u1.id_node.name));
			break;
		} 
		case AST_NODE_TYPE_POSTFIX: {
			switch(p->u1.postfix_node.type) {
				case AST_POSTFIX_OP_INC: {
					como_emit(CG(op_array), POSTFIX_INC, newString(p->u1.postfix_node.expr->u1.id_node.name));
					break;
				}
				case AST_POSTFIX_OP_DEC: {
					como_emit(CG(op_array), POSTFIX_DEC, newString(p->u1.postfix_node.expr->u1.id_node.name));
					break;
				}
			}
			break;
		}
		case AST_NODE_TYPE_UNARY_OP: {
			switch(p->u1.unary_node.type) {
				case AST_UNARY_OP_MINUS: {
					como_compile(p->u1.unary_node.expr);
					como_emit(CG(op_array), UNARY_MINUS, NULL);
					break;
				}
			}
		}
		break;
		case AST_NODE_TYPE_BIN_OP: {
			switch(p->u1.binary_node.type) {
				case AST_BINARY_OP_REM:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IREM, NULL);
				break;	
				case AST_BINARY_OP_LTE:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_LESS_THAN_OR_EQUAL, NULL);
				break;	
				case AST_BINARY_OP_GTE:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_GREATER_THAN_OR_EQUAL, NULL);
				break;
				case AST_BINARY_OP_LT: 
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_LESS_THAN, NULL);
				break;
				case AST_BINARY_OP_GT:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_GREATER_THAN, NULL);
				break;
				case AST_BINARY_OP_CMP:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_EQUAL, NULL);
				break;
				case AST_BINARY_OP_NEQ:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IS_NOT_EQUAL, NULL);
				break;
				case AST_BINARY_OP_MINUS: 
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IMINUS, NULL);
				break;
				case AST_BINARY_OP_DIV:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IDIV, NULL);
				break;
				case AST_BINARY_OP_ADD:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),IADD, NULL);
				break;
				case AST_BINARY_OP_TIMES:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array), ITIMES, NULL);
				break;
				case AST_BINARY_OP_ASSIGN: 
					como_compile(p->u1.binary_node.right);
					como_emit(CG(op_array),STORE_NAME, newString(p->u1.binary_node.left->u1.id_node.name));			
				break;
			}	
		} break;
	}
	return 0;
}

void como_ast_create(const char *filename, const char* text)
{
	como_op_array *op_array = como_op_array_create();

	if((ceg = como_executor_globals_create(filename, op_array)) == NULL) {
		como_error_noreturn("como_executor_globals_created failed");
	}

	Object *call_info = newArray(3);
	arrayPushEx(call_info, newString("__main__"));
	arrayPushEx(call_info, newLong(0L));
	arrayPushEx(call_info, newLong(0L));

	como_stack_push(&CG(op_array)->call_stack, (void *)call_info);
	/* Push this symbol table */
	Object *firstsymtab = CG(op_array)->frame->cf_symtab;
	como_stack_push(&CG(scope), (void *)firstsymtab);


	ast_node* statements;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	
	if(yylex_init(&scanner)) {
		como_error_noreturn("yylex_init returned NULL");
	}

	state = yy_scan_string(text, scanner);

	if(yyparse(&statements, scanner)) {
		como_error_noreturn("yyparse returned NULL");
	}

	yy_delete_buffer(state, scanner);

	yylex_destroy(scanner);

	como_compile(statements);

	como_emit(CG(op_array), HALT, NULL);

	(void)como_vm(CG(op_array));
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

	como_ast_create(argv[1], text);

	free(text);

	return 0;
}





