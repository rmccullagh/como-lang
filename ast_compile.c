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

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <object.h>
#include "ast.h"
#include "stack.h"
#include "globals.h"
#include "como_opcode.h"

static void como_print_stack_trace(void);
static void debug_code_to_output(FILE *);
#define COMO_COMPILER 1 
#include "comodebug.h"

#define CF_STACKSIZE 1024
#define COMO_DEFAULT_OP_ARRAY_CAPACITY 512

typedef struct como_frame {
		size_t cf_sp;
		Object *cf_fname;
		Object *cf_stack[CF_STACKSIZE];
		Object *cf_symtab;
		Object *cf_retval;
} como_frame;

static int como_frame_init(como_frame *frame) {
	frame->cf_sp = 0;
	frame->cf_fname = NULL;
	frame->cf_symtab = newMap(2);
	frame->cf_retval = NULL;

	if(frame->cf_symtab == NULL) 
		return COMO_FAILURE;
	return COMO_SUCCESS;
}

#define PUSH(e) do { \
		if(cframe->cf_sp + 1 >= CF_STACKSIZE) { \
					printf("error stack overflow tried to push onto #%zu\n", cframe->cf_sp + 1); \
					exit(1); \
				} \
		cframe->cf_stack[cframe->cf_sp++] = e; \
} while(0)

#define POP(n) do { \
		if(cframe->cf_sp == 0) { \
			n = cframe->cf_stack[0]; \
		} else { \
			n = cframe->cf_stack[--cframe->cf_sp]; \
		} \
} while(0)

#define OBJ_DECREF(o) do { \
	if(--O_REFCNT((o)) == 0) { \
		objectDestroy(o); \
	} \
} while(0)

typedef struct como_i {
	unsigned char opcode;
} como_i;

typedef struct op_code {
	struct como_i inst;
	Object *op1;
} op_code;

typedef struct op_array {
	size_t size;
	size_t capacity;
	op_code **table;
} op_array;

static int op_array_init(op_array *o) {
	o->size = 0;
	o->capacity = COMO_DEFAULT_OP_ARRAY_CAPACITY;
	o->table = calloc(COMO_DEFAULT_OP_ARRAY_CAPACITY, 
		sizeof(op_code));

	if(o->table == NULL) 
		return COMO_FAILURE;

	return COMO_SUCCESS;
}


typedef struct compiler_context {
	size_t pc;
	op_array code;
	Object *function_table;
} compiler_context;

typedef struct como_function {
	size_t fn_arg_count;
	Object *arguments;
	compiler_context *fn_ctx;
	como_frame *fn_frame;
} como_function;


typedef struct ex_globals {
	Object *filename;
	como_stack* function_call_stack;
	como_stack *frame_stack;
} ex_globals;


static ex_globals *eg;
static compiler_context *cg;
static como_frame *cframe;

static void como_print_stack_trace(void)
{
	como_stack* top = eg->function_call_stack;
	while(top != NULL) {
		Array* call_info = O_AVAL(((Object*)(top->value)));
		fprintf(stderr, "  at %s (%s:%ld:%ld)\n", 
			O_SVAL(call_info->table[0])->value, O_SVAL(eg->filename)->value, 
			O_LVAL(call_info->table[1]), 
			O_LVAL(call_info->table[2]));

		top = top->next;
	}
}

static void call_stack_push(Object *fname, Object *lineno, Object *colno)
{
	Object *call_info = newArray(3);
	arrayPushEx(call_info, fname);
	arrayPushEx(call_info, lineno);
	arrayPushEx(call_info, colno);

	como_stack_push(&eg->function_call_stack, (void *)call_info);
}

static void frame_stack_push(compiler_context *f) {
	como_stack_push(&eg->frame_stack, (void *)f);
}

static void executor_init(const char *filename, ex_globals *e) {
	e->filename = newString(filename);
	e->function_call_stack = NULL;
	e->frame_stack = NULL;

	Object *call_info = newArray(3);
	arrayPushEx(call_info, newString("__main__"));
	arrayPushEx(call_info, newLong(0L));
	arrayPushEx(call_info, newLong(0L));

	como_stack_push(&e->function_call_stack, (void *)call_info);
}


static int compiler_context_init(compiler_context *ctx)
{
	ctx->pc = 0;
	ctx->function_table = newMap(4);
	if(op_array_init(&ctx->code) == COMO_FAILURE)
		return COMO_FAILURE;
	return COMO_SUCCESS;
}


#define BINARY_OP_SETUP \
		Object *left; \
		Object *right; \
		POP(right); \
		POP(left); \

static void debug_code_to_output(FILE *fp) {
		
		fprintf(fp, "*** BEGIN CODE ***\n");
		size_t i;
		for(i = 0; i < cg->code.size; i++) {
			op_code *c = cg->code.table[i];
			switch(c->inst.opcode) {
				case IREM: {
					fprintf(fp, "\tIREM\n");
					break;
				}
				case IRETURN: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tIRETURN %s\n", value);
					free(value);
					break;
				}
				case CALL_FUNCTION: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tCALL_FUNCTION %s\n", value);
					free(value);
					break;
				}
				case DEFINE_FUNCTION: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tDEFINE_FUNCTION %s\n", value);
					free(value);
					break;
				}
				case LOAD_CONST: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tLOAD_CONST %s\n", value);
					free(value);
					break;
				}
				case STORE_NAME: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tSTORE_NAME %s\n", value);
					free(value);
					break;
				}
				case LOAD_NAME: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tLOAD_NAME %s\n", value);
					free(value);
					break;
				}
				case IS_LESS_THAN: {
					fprintf(fp, "\tIS_LESS_THAN\n");
					break;
				}
				case JZ: {
					char *value = objectToString(c->op1);
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
					char *value = objectToString(c->op1);
					fprintf(fp, "\tJMP %s\n", value);
					free(value);
					break;
				}
				case NOP: {
					fprintf(fp,"\tNOP\n");
					break;
				}
				case LABEL: {
					char *value = objectToString(c->op1);
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
			}
		}
		fprintf(fp, "*** END CODE ***\n");
}

#define ENSURE_NUMERIC_OPERANDS \
	if(O_TYPE(left) != IS_LONG && O_TYPE(right) != IS_LONG) { \
		fprintf(stderr, "como: can't invoke binary operation on non numeric arguments\n"); \
		exit(1); \
	} \

#define TARGET(x) \
	case x: \

#define VM_CONTINUE \
	cg->pc++; \
	break; \

#define OPERANDS_ARE_BOTH_NUMERIC \
	(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG)

#define BINARY_OP_FREE
#define FREE_OP(x)

static void como_vm(void);
static void emit(unsigned char, Object *);

static void como_vm(void) {

	if(getenv("DEBUG")) {
		debug_code_to_output(stdout);
		return;
	}

	while(1) {
		size_t pc = cg->pc;
		
		if(!(pc < cg->code.size)) {
			fprintf(stderr, "pc is passed the size, pc=%zu, size=%zu\n",
				pc, cg->code.size);
			assert(0);
		}

		op_code *c = cg->code.table[pc];
		assert(c);
		
		switch(c->inst.opcode) {
			TARGET(POSTFIX_INC) {
				Object *name = mapSearchEx(cframe->cf_symtab, O_SVAL(c->op1)->value);
				if(name == NULL) {
					fprintf(stderr, "Undefined variable %s\n", O_SVAL(c->op1)->value);
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
				OBJ_DECREF(c->op1);
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
				if(O_LVAL(c->op1) == 0L) {
					como_debug("returning from function without return value");
					cframe->cf_retval = newLong(0L);
				} else {
					POP(cframe->cf_retval);
				}
				return;
			}
			TARGET(DEFINE_FUNCTION) {
				VM_CONTINUE
			}
			TARGET(LOAD_CONST) {
				PUSH(c->op1);
				VM_CONTINUE
			}
			TARGET(STORE_NAME) {
				Object *v;
				POP(v);
				/* make sure to copy the value of v 
				   because v might be coming from a variable
				 */
				mapInsertEx(cframe->cf_symtab, O_SVAL(c->op1)->value, copyObject(v));
				PUSH(v);
				VM_CONTINUE
			}
			TARGET(LOAD_NAME) {
				Object *v = mapSearchEx(cframe->cf_symtab, O_SVAL(c->op1)->value);
				if(v == NULL) {
					fprintf(stderr, "Undefined variable %s\n", O_SVAL(c->op1)->value);
					PUSH(newLong(0L));
				} else {
					PUSH(v);
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
					cg->pc = O_LVAL(c->op1);		
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
				cg->pc = O_LVAL(c->op1);
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
				PUSH(newLong(
					(long)
						(objectValueCompare(left, right) ||
						 	objectValueIsLessThan(left, right)
						)
					)
				);
				VM_CONTINUE		
			}
			TARGET(CALL_FUNCTION) {
				Object *code = NULL;
				como_stack* top = eg->frame_stack;
				como_stack *prev = NULL;
				compiler_context *tmpctx = NULL;
				while(top != NULL) {
					tmpctx = (compiler_context *)top->value;
					code = mapSearchEx(tmpctx->function_table, O_SVAL(c->op1)->value);
					if(code != NULL) {
						break;
					}
					top = top->next;
					prev = top;
				}
				
				if(code == NULL) {
					como_error_noreturn("call to undefined function '%s'", O_SVAL(c->op1)->value);
				}

				Object *total_args;
				Object *colno;
				Object *lineno;

				POP(total_args);
				POP(colno);
				POP(lineno);

				como_function *fn;
				ssize_t i;

				fn = (como_function *)O_PTVAL(code);
				Array *arguments = O_AVAL(fn->arguments);

				if(fn->fn_arg_count != (size_t)O_LVAL(total_args)) {
					como_error_noreturn("function '%s' expects exactly %zu arguments, %ld given",
						O_SVAL(c->op1)->value, fn->fn_arg_count, O_LVAL(total_args));
				}

				i = (ssize_t)fn->fn_arg_count;
				while(i--) {
					Object *arg_value;
					POP(arg_value);
					Object *arg_name = arguments->table[i];
					/* It's important to copy the value of arg_value (mapInsert) here
					   as, if we don't, then arguments will actually be passed by reference, not value
					 */
					mapInsert(fn->fn_frame->cf_symtab, O_SVAL(arg_name)->value, 
						arg_value);
				}

				/* save context */
				compiler_context *old_ctx = cg;
				como_frame *_old_cframe = cframe;

				como_debug("calling function '%s' with %ld arguments", O_SVAL(c->op1)->value,
					O_LVAL(total_args));


				cg = fn->fn_ctx;
				cframe = fn->fn_frame;

				if(cg->code.table[cg->code.size - 1]->inst.opcode != IRETURN) {
					como_debug("inserting IRETURN for function %s", O_SVAL(c->op1)->value);
					emit(LOAD_CONST, newLong(0L));
					emit(IRETURN, newLong(1L));
				}

				call_stack_push(c->op1, lineno, colno);
				/*
				 * push this frame onto the frame stack
				 */
				frame_stack_push(fn->fn_ctx);

				//return;
				(void)como_vm();

				Object *retval = copyObject(cframe->cf_retval);

				como_debug("stack pointer is at %zu", cframe->cf_sp);
				
				/* restore context */
				cg = old_ctx;
				cframe = _old_cframe;
				
				/* BEGIN RESET */
				fn->fn_frame->cf_retval = NULL;
				objectDestroy(fn->fn_frame->cf_symtab);
				fn->fn_frame->cf_symtab = newMap(2);
				fn->fn_frame->cf_sp = 0;
				fn->fn_ctx->pc = 0;
				size_t ii;
				for(ii = 0; ii < CF_STACKSIZE; ii++) {
					fn->fn_frame->cf_stack[ii] = NULL;
				}
				/* END RESET */

				/*
				 * TODO pop the frame_stack and kill all objects in it 
				 */
				 
				if(prev) {
					como_debug("popping frame stack");
					prev->next = top->next;
				} else {
					eg->frame_stack = top->next;
				}
			
				PUSH(retval);
				VM_CONTINUE
			}
		}
	}
}

static void emit(unsigned char op, Object *arg) {
	op_code *i = malloc(sizeof(op_code));
	como_i inst;
	inst.opcode = op;
	i->inst = inst;
	i->op1 = arg;
	if(cg->code.size >= cg->code.capacity) {
		assert(0);
	}
	cg->code.table[cg->code.size++] = i;
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
			emit(LOAD_CONST, newString(p->u1.string_value.value));
		break;
		case AST_NODE_TYPE_PRINT:
			como_compile(p->u1.print_node.expr);
			emit(IPRINT, NULL);
		break;
		case AST_NODE_TYPE_NUMBER:
			emit(LOAD_CONST, newLong(p->u1.number_value));
		break;
		case AST_NODE_TYPE_ID:
			emit(LOAD_NAME, newString(p->u1.id_node.name));
		break;
		case AST_NODE_TYPE_RET:
			if(p->u1.return_node.expr != NULL) {
				como_compile(p->u1.return_node.expr);
				emit(IRETURN, newLong(1L));
			} else {
				emit(IRETURN, newLong(0L));
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
			Object *l = newLong(cg->code.size);
			/**
			 * The marker of the start of the while loop
			 */
			emit(LABEL, l);
			
			Object *l2 = newLong(0);
			como_compile(p->u1.while_node.condition);
		
			/**
			 * Need to back patch this to an operand address that
			 * has yet to be seen
			 */	
			emit(JZ, l2);


			como_compile(p->u1.while_node.body);
			
			emit(JMP, newLong(O_LVAL(l)));
    	
			Object *l3 = newLong(cg->code.size);
    	
			/**
			 * Now go back to the first label generated here (in this case )
			 */
			
			/**
			 * The upper JZ is targeted to this instruction
			 */
			emit(LABEL, l3);
    	
			O_LVAL(l2) = O_LVAL(l3);
		
		}
		break;
		case AST_NODE_TYPE_IF: {
			Object *l2 = newLong(0);
			Object *l4 = newLong(0);
			como_compile(p->u1.if_node.condition);
		
			emit(JZ, l2);

			como_compile(p->u1.if_node.b1);

			emit(JMP, l4);

			
			Object *l3 = newLong(cg->code.size);
    	
    		emit(LABEL, l3);

			if(p->u1.if_node.b2 != NULL) {
				como_compile(p->u1.if_node.b2);
			}
    	
			O_LVAL(l2) = O_LVAL(l3);
			O_LVAL(l4) = cg->code.size;

			emit(LABEL, newLong(cg->code.size));

		} 
		break;
		case AST_NODE_TYPE_FUNC_DECL: {	
			emit(DEFINE_FUNCTION, newString(p->u1.function_node.name));

			/* save context */
			compiler_context *old_ctx = cg;
			como_frame *_old_cframe = cframe;

			compiler_context *ctx_fn = malloc(sizeof(compiler_context));
			como_frame *_cframe_fn = malloc(sizeof(como_frame));

			if(compiler_context_init(ctx_fn) == COMO_FAILURE) {
				como_error_noreturn("failed to initialize compiler context for function def.");
			}
			
			cg = ctx_fn;

			if(como_frame_init(_cframe_fn) == COMO_FAILURE) {
				como_error_noreturn("failed to aquire execution frame for function defn.");
			}

	
			cframe = _cframe_fn;
			como_function *fn = malloc(sizeof(como_function));
			fn->arguments = newArray(2);

			size_t i;
			for(i = 0; i < p->u1.function_node.parameter_list->u1.statements_node.count; i++) {
				ast_node *arg = p->u1.function_node.parameter_list->u1.statements_node.statement_list[i];
				assert(arg->type == AST_NODE_TYPE_ID);
				arrayPushEx(fn->arguments, newString(arg->u1.id_node.name));
			}

			como_compile(p->u1.function_node.body);

			fn->fn_arg_count = p->u1.function_node.parameter_list->u1.statements_node.count;
			fn->fn_ctx = ctx_fn;
			fn->fn_frame = _cframe_fn;

			mapInsertEx(old_ctx->function_table, p->u1.function_node.name, newPointer((void *)fn));
			/* restore context */
			cg = old_ctx;
			cframe = _old_cframe;

		} 
		break;
		case AST_NODE_TYPE_CALL: {
			como_compile(p->u1.call_node.arguments);
			emit(LOAD_CONST, newLong((long)p->u1.call_node.lineno));
			emit(LOAD_CONST, newLong((long)p->u1.call_node.colno));
			emit(LOAD_CONST, newLong(p->u1.call_node.arguments->u1.statements_node.count));
			emit(CALL_FUNCTION, newString(p->u1.call_node.id->u1.id_node.name));
			break;
		} 
		case AST_NODE_TYPE_UNARY_OP: {
			switch(p->u1.unary_node.type) {
				case AST_UNARY_OP_POSTFIX_INC: {
					emit(POSTFIX_INC, newString(p->u1.unary_node.expr->u1.id_node.name));
					break;
				}
				case AST_UNARY_OP_MINUS: {
					como_compile(p->u1.unary_node.expr);
					emit(UNARY_MINUS, NULL);
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
					emit(IREM, NULL);
				break;	
				case AST_BINARY_OP_LTE:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_LESS_THAN_OR_EQUAL, NULL);
				break;	
				case AST_BINARY_OP_GTE:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_GREATER_THAN_OR_EQUAL, NULL);
				break;
				case AST_BINARY_OP_LT: 
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_LESS_THAN, NULL);
				break;
				case AST_BINARY_OP_GT:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_GREATER_THAN, NULL);
				break;
				case AST_BINARY_OP_CMP:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_EQUAL, NULL);
				break;
				case AST_BINARY_OP_NEQ:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_NOT_EQUAL, NULL);
				break;
				case AST_BINARY_OP_MINUS: 
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IMINUS, NULL);
				break;
				case AST_BINARY_OP_DIV:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IDIV, NULL);
				break;
				case AST_BINARY_OP_ADD:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IADD, NULL);
				break;
				case AST_BINARY_OP_TIMES:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(ITIMES, NULL);
				break;
				case AST_BINARY_OP_ASSIGN: 
					como_compile(p->u1.binary_node.right);
					emit(STORE_NAME, newString(p->u1.binary_node.left->u1.id_node.name));			
				break;
			}	
		} break;
	}
	return 0;
}


void ast_compile(const char *filename, ast_node* program)
{
	assert(program);

	compiler_context ctx;
	como_frame _cframe;
	ex_globals _exg;

	if(compiler_context_init(&ctx) == COMO_FAILURE) {
		como_error_noreturn("failed to initialize compiler context");
	}

	
	cg = &ctx;
	cframe = &_cframe;
	eg = &_exg;

	executor_init(filename, &_exg);

	(void )como_compile(program);
	emit(HALT, NULL);
	

	if(como_frame_init(&_cframe) == COMO_FAILURE) {
		como_error_noreturn("failed to aquire execution frame");
	}

	frame_stack_push(&ctx);


	ast_node_free(program);

	como_vm();

		OBJECT_DUMP_EX(cframe->cf_symtab);

}
