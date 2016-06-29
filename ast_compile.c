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

#define C_USER_FUNC (1 << 0)
#define C_EXT_FUNC  (1 << 1)

typedef void(*como_ext_func)(Object*, Object**);

static void debug(const char* format, ...)
{
#ifdef COMO_DEBUG
	fprintf(stderr, "DEBUG: ");
	va_list args;
	va_start (args, format);
	vfprintf (stderr, format, args);
	va_end (args);
#endif
}

#define CF_STACKSIZE 1024

typedef struct como_frame {
		Object *cf_symtab;
		Object *cf_stack[CF_STACKSIZE];
		size_t cf_sp;
} como_frame;

static como_frame cframe;

static int como_frame_init(como_frame *frame) {
	frame->cf_symtab = newMap(2);
	frame->cf_sp = 0;
	return 1;
}

#define PUSH(e) do { \
		if(cframe.cf_sp + 1 >= CF_STACKSIZE) { \
					printf("error stack overflow tried to push onto #%zu\n", cframe.cf_sp + 1); \
					exit(1); \
				} \
		cframe.cf_stack[cframe.cf_sp++] = e; \
} while(0)

#define POP(n) do { \
		if(cframe.cf_sp - 1 == SIZE_MAX) { \
					printf("%s:%d: error stack underflow, tried to go before zero: %zu\n", \
							__func__, __LINE__, cframe.cf_sp); \
					exit(1); \
				} \
		n = cframe.cf_stack[--cframe.cf_sp]; \
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

#define MAX_LABELS 1024

static int should_only_show_code = 0;

typedef struct compiler_context {
	int lbl;
	size_t label_table[MAX_LABELS];
	size_t label_total;
	Object* filename;
	const char* active_function_name;
	Object* symbol_table;
	Object* current_symbol_table;
	como_stack* function_call_stack;
	como_stack *global_call_stack;
	Object* return_value;
	size_t pc;
	op_array *code;
} compiler_context;

static compiler_context* cg;

static void como_var_dump(Object* args, Object** retval)
{
	printf("%s()\n", __func__);
	OBJECT_DUMP(args);
	*retval = NULL;
	cg->return_value = NULL;
}

#define LOAD_CONST   0x01
#define STORE_NAME   0x02
#define LOAD_NAME    0x03
#define IS_LESS_THAN 0x04
#define JZ			     0x05
#define IPRINT       0x06
#define IADD         0x45
#define JMP          0x52
#define IRETURN      0xff
#define NOP          0x42
#define LABEL        0x0b
#define HALT         0x0c
#define IS_EQUAL     0x0d
#define IDIV         0x0e

#define DEBUG_OBJECT(o) do { \
	fprintf(stdout, "DEBUGGING OBJECT:\n\t"); \
	OBJECT_DUMP((o)); \
	fprintf(stdout, "\n"); \
	fflush(stdout); \
} while (0) 

#define BINARY_OP_SETUP \
		Object *left; \
		Object *right; \
		POP(right); \
		POP(left); \

static void debug_code_to_output(FILE *fp) {
		
		fprintf(fp, "*** BEGIN CODE ***\n");
		size_t i;
		for(i = 0; i < cg->code->size; i++) {
			op_code *c = cg->code->table[i];
			switch(c->inst.opcode) {
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
				case IDIV: {
					fprintf(fp, "\tDIV\n");
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
			}
		}
		fprintf(fp, "*** END CODE ***\n");
}

static void como_vm() {
	
	//debug_code_to_output(stdout);

	//return;

	while(1) {
		size_t pc = cg->pc;
		if(!(pc < cg->code->size)) {
			fprintf(stderr, "pc is passed the size, pc=%zu, size=%zu\n",
				pc, cg->code->size);
			assert(0);
		}

		op_code *c = cg->code->table[pc];
		
#ifdef DEBUG
		fprintf(stderr, "pc=%zu\n", pc);
#endif
			assert(c);


		switch(c->inst.opcode) {
			case LOAD_CONST: {
				PUSH(c->op1);
				cg->pc++;
				break;
			}
			case STORE_NAME: {
				Object *v;
				POP(v);
				mapInsert(cframe.cf_symtab, O_SVAL(c->op1)->value, v);
				PUSH(v);
				cg->pc++;
				break;
			}
			case LOAD_NAME: {
				Object *v = mapSearch(cframe.cf_symtab, O_SVAL(c->op1)->value);
				if(v == NULL) {
					fprintf(stderr, "Undefined variable %s\n", O_SVAL(c->op1)->value);
					PUSH(newLong(0));
				} else {
					PUSH(v);
				}
				cg->pc++;
				break;
			}
			case IS_LESS_THAN: {
				BINARY_OP_SETUP
				if(objectValueIsLessThan(left, right)) {
					PUSH(newLong(1));
				} else {
					PUSH(newLong(0));
				}
				cg->pc++;
				break;
			}
			case JZ: {
				Object *cond;
				POP(cond);
				if(O_TYPE(cond) == IS_LONG && O_LVAL(cond) == 0) {
					cg->pc = O_LVAL(c->op1);		
					break;			
				} else {
					cg->pc++;
				}
				break;
			}
			case IPRINT: {
				Object *value;
				POP(value);
				size_t len = 0;
				char *sval = objectToStringLength(value, &len);
				fprintf(stdout, "%s\n", sval);
				free(sval);
				cg->pc++;
				break;
			}
			case IADD: {
				BINARY_OP_SETUP
				if(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG) {
					PUSH(newLong(O_LVAL(left) + O_LVAL(right)));
				}	else {
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
				cg->pc++;
				break;
			}
			case IDIV: {
				BINARY_OP_SETUP
				if(O_TYPE(left) != IS_LONG && O_TYPE(right) != IS_LONG) {
					fprintf(stderr, "como: Can't divide non numeric objects\n");
					exit(1);
				}
				break;	
			}
			case JMP: {
				cg->pc = O_LVAL(c->op1);
				break;
			}
			case NOP: {
				cg->pc++;
				break;
			}
			case LABEL: {
				cg->pc++;
				break;
			}
			case HALT: {
				return;
			}
			case IS_EQUAL: {
				BINARY_OP_SETUP
				if(objectValueCompare(left, right)) {
					PUSH(newLong(1)) ;
				} else {
					PUSH(newLong(0));
				}
				cg->pc++;
				break;
			}
		}
	}
}


static void compiler_init(void)
{
	cg = malloc(sizeof(compiler_context));
	cg->lbl = 0;
	cg->pc = 0;
	cg->code = malloc(sizeof(op_array));
	cg->code->size = 0;
	cg->code->capacity = 1024;
	cg->code->table = calloc(64, sizeof(op_code));


	cg->label_total = 0;
	cg->symbol_table = newMap(2);
	cg->current_symbol_table = NULL;
	cg->function_call_stack = NULL;
	cg->global_call_stack = NULL;
	cg->return_value = NULL;
	cg->active_function_name = "__main__";
	cg->filename = NULL;

	Object* var_dump = newFunction(como_var_dump);	
	O_MRKD(var_dump) = C_EXT_FUNC;
	mapInsert(cg->symbol_table, "var_dump", var_dump);
	objectDestroy(var_dump);

}

static compiler_context* cg_context_create()
{
	compiler_context* ctx = malloc(sizeof(compiler_context));
	ctx->symbol_table = NULL;
	ctx->current_symbol_table = NULL;
	ctx->function_call_stack = NULL;
	ctx->return_value = NULL;
	ctx->active_function_name = "<unknown>";	
	return ctx;	
}

void dump_fn_call_stack(void)
{
	como_stack* top = cg->function_call_stack;
	while(top != NULL) {
		Array* call_info = O_AVAL(((Object*)(top->value)));
		printf("  at %s (%s:%ld:%ld)\n", 
			O_SVAL(call_info->table[0])->value, O_SVAL(cg->filename)->value, 
			O_LVAL(call_info->table[1]), 
			O_LVAL(call_info->table[2]));

		top = top->next;
	}
}

static void como_print_stack_trace(void)
{
	como_stack* top = cg->global_call_stack;
	while(top != NULL) {
		Array* call_info = O_AVAL(((Object*)(top->value)));
		printf("  at %s (%s:%ld:%ld)\n", 
			O_SVAL(call_info->table[0])->value, O_SVAL(cg->filename)->value, 
			O_LVAL(call_info->table[1]), 
			O_LVAL(call_info->table[2]));

		top = top->next;
	}
}

static Object* ex(ast_node* p);

static int is_truthy(Object* o)
{
	if(!o)
		return 0;
	if(O_TYPE(o) == IS_LONG) {
		return O_DVAL(o) ? 1 : 0;
	}
	if(O_TYPE(o) == IS_NULL) {
		return 0;
	}	
	if(O_TYPE(o) == IS_BOOL) {
		return O_BVAL(o) ? 1 : 0;
	}

	return 0;
}

static Object* call_info_create(const char* name, int line, int col)
{
	Object *call_info = newArray(3);
	
	Object* fname = newString(name);
	arrayPush(call_info, fname);
	objectDestroy(fname);

	Object* lineno = newLong(line);
	arrayPush(call_info, lineno);
	objectDestroy(lineno); 

	Object* colno = newLong(col);
	arrayPush(call_info, colno);
	objectDestroy(colno);

	return call_info;
}

static void call_stack_push(Object *call_info)
{
	como_stack_push(&cg->function_call_stack, call_info);
}

static void como_stack_push_ex(como_stack **stack, Object *call_info)
{
	como_stack_push(stack, call_info);
}


static void emit(unsigned char op, Object *arg) {
	op_code *i = malloc(sizeof(op_code));
	como_i inst;
	inst.opcode = op;
	i->inst = inst;
	i->op1 = arg;
	if(cg->code->size >= cg->code->capacity) {
		assert(0);
	}

	cg->code->table[cg->code->size++] = i;
}

static int como_compile(ast_node* p)
{
	int lbl1, lbl2;

	if(!p)
		return 0;

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
			emit(IPRINT, newNull());
		break;
		case AST_NODE_TYPE_NUMBER:
			emit(LOAD_CONST, newLong(p->u1.number_value));
		break;
		case AST_NODE_TYPE_ID:
			emit(LOAD_NAME, newString(p->u1.id_node.name));
		break;
		case AST_NODE_TYPE_RET:
			emit(IRETURN, newNull());
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
				ast_node* stmt = p->u1.statements_node.statement_list[i];
				como_compile(stmt);
			}
		} break;
		case AST_NODE_TYPE_WHILE: {
			Object *l = newLong(cg->code->size);
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
			//printf("\tjmp\tL%03d\n", O_LVAL(l));
			emit(JMP, newLong(O_LVAL(l)));
    	//printf("L%03d:\n", cg->code->size);
    	Object *l3 = newLong(cg->code->size);
    	
			/**
			 * Now go back to the first label generated here (in this case )
			 */
			
			cg->label_table[cg->label_total++] = cg->code->size - 1;
			/**
			 * The upper JZ is targeted to this instruction
			 */
			emit(LABEL, l3);
    	
			O_LVAL(l2) = O_LVAL(l3);
		
		}
		break;
		case AST_NODE_TYPE_IF: {
		} 
		break;
		case AST_NODE_TYPE_FUNC_DECL: {	
		} 
		break;
		case AST_NODE_TYPE_CALL: {

		} 
		break;
		case AST_NODE_TYPE_BIN_OP: {
			switch(p->u1.binary_node.type) {
				default:
					printf("%s(): invalid binary op(%d)\n", __func__, p->u1.binary_node.type);
					exit(1);
				break;
				case AST_BINARY_OP_LTE: {
				}
				break;
				case AST_BINARY_OP_LT: {
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_LESS_THAN, newNull());
				}
				break;
				case AST_BINARY_OP_GT: {
				}
				break;
				case AST_BINARY_OP_CMP: {
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IS_EQUAL, newNull());
				}
				break;
				case AST_BINARY_OP_MINUS: {
				}
				break;
				case AST_BINARY_OP_DIV:
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IDIV, newNull());
				break;
				case AST_BINARY_OP_ADD: {
					como_compile(p->u1.binary_node.left);
					como_compile(p->u1.binary_node.right);
					emit(IADD, newNull());
				}
				break;
				case AST_BINARY_OP_TIMES: {
				} 
				break;
				case AST_BINARY_OP_ASSIGN: {
					const char* id = p->u1.binary_node.left->u1.id_node.name;
					como_compile(p->u1.binary_node.right);
					emit(STORE_NAME, newString(id));			
				} 
				break;
			}	
		} break;
	}
	return 0;
}


void ast_compile(const char* filename, ast_node* program)
{
	if(!program) {
		printf("%s(): unexpected empty node\n", __func__);
		exit(1);
	}

	compiler_init();

	como_stack_push_ex(&cg->global_call_stack, call_info_create(
 		"__main__", 0, 0
  ));

	cg->filename = newString(filename);	
	
	//Object* ret = ex(program);
	(void )como_compile(program);
	emit(HALT, newNull());

	como_frame_init(&cframe);
	como_vm();

	objectDestroy(cg->symbol_table);
	//objectDestroy(ret);
	objectDestroy(cg->filename);
	
	como_stack* top = cg->function_call_stack;
	while(top != NULL) {
		como_stack* next = top->next;
		objectDestroy(((Object*)(top->value)));
		free(top);	
		top = next;
	}

	free(cg);	
}
