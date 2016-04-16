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
#include <stdio.h>
#include <object.h>
#include "ast.h"
#include "stack.h"
#include "globals.h"
#include "object_api.h"

#define C_USER_FUNC (1 << 0)
#define C_EXT_FUNC  (1 << 1)
#define C_VALUE_CONST (1 << 2)

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

typedef struct compiler_context {
	Object* filename;
	const char* active_function_name;
	Object* symbol_table;
	Object* current_symbol_table;
	como_stack* function_call_stack;
	como_stack *global_call_stack;
	Object* return_value;
} compiler_context;

static compiler_context* cg;

static void como_var_dump(Object* args, Object** retval)
{
	printf("%s()\n", __func__);
	OBJECT_DUMP(args);
	*retval = NULL;
	cg->return_value = NULL;
}

static void compiler_init(void)
{
	cg = malloc(sizeof(compiler_context));
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


static Object* ex(ast_node* p)
{
	if(!p)
		return NULL;

	switch(p->type) {
		default:
			printf("%s(): invalid node type(%d)\n", __func__, p->type);
			exit(1);
		break;
		case AST_NODE_TYPE_STRING: {
			return newString(p->u1.string_value.value);
		}
		break;
		case AST_NODE_TYPE_PRINT: {
			Object* expr = ex(p->u1.print_node.expr);
			objectEcho(expr);
			objectDestroy(expr);
			fprintf(stdout, "\n");
			return newNull();
		}
		break;
		case AST_NODE_TYPE_NUMBER:
			return newLong(p->u1.number_value);
		break;
		case AST_NODE_TYPE_DOUBLE:
			return newDouble(p->u1.double_value);
		break;
		case AST_NODE_TYPE_ID: {
			Object* value;
			if(cg->current_symbol_table == NULL) {
				value = mapSearch(cg->symbol_table, p->u1.id_node.name);
			} else {
				value = mapSearch(cg->current_symbol_table, p->u1.id_node.name);
				if(!value) {
					value = mapSearch(cg->symbol_table, p->u1.id_node.name);
				}
			}
			if(!value) {
				printf("warning: undefined variable %s\n", p->u1.id_node.name);
				como_print_stack_trace();
				return newLong(0);
			} else {
				return value;
			}
		}
		break;
		case AST_NODE_TYPE_RET: {
			if(p->u1.return_node.expr != NULL) {
				cg->return_value = ex(p->u1.return_node.expr);
			} else {
				cg->return_value = newNull();
			}
			return newNull();
		} break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
				ast_node* stmt = p->u1.statements_node.statement_list[i];
				Object* ret = ex(stmt);
				objectDestroy(ret);
				if(cg->return_value != NULL) {
					return newNull();
				}
			}
			return newNull();
		} break;
		case AST_NODE_TYPE_WHILE: {
			Object* cond = ex(p->u1.while_node.condition);

			while(is_truthy(cond)) {
				ex(p->u1.while_node.body);	
				cond = ex(p->u1.while_node.condition);
			}
			return newNull();	
		}
		break;
		case AST_NODE_TYPE_IF: {
			Object* cond = ex(p->u1.if_node.condition);

			if(is_truthy(cond)) {
				objectDestroy(cond);
				return ex(p->u1.if_node.b1);	
			} else {
				objectDestroy(cond);
				if(p->u1.if_node.b2) {
					return ex(p->u1.if_node.b2);
				}
			}
			return newNull();	
		} break;
		case AST_NODE_TYPE_FUNC_DECL: {
			const char* id = p->u1.function_node.name;
			Object* fn = newFunction((void*)p);
			O_MRKD(fn) = C_USER_FUNC;

			if(cg->current_symbol_table == NULL) {
				mapInsert(cg->symbol_table, id, fn);
			} else {
				mapInsert(cg->current_symbol_table, id, fn);
			}
			objectDestroy(fn);
			return newNull();	
		} break;
		case AST_NODE_TYPE_CALL: {
			const char* id = p->u1.call_node.id->u1.id_node.name;
			int fcall_lineno = p->u1.call_node.lineno;
			int fcall_colno = p->u1.call_node.colno;

			Object* fn;
			if(cg->current_symbol_table == NULL) {
				debug("searching global symbol table for id \"%s\" in scope \"%s\"\n", 
						id, cg->active_function_name);
				fn = mapSearch(cg->symbol_table, id);
			} else {
				debug("searching current symbol table for id \"%s\" in scope \"%s\"\n", 
						id, cg->active_function_name);
				fn = mapSearch(cg->current_symbol_table, id);
				if(!fn) {
					fn = mapSearch(cg->symbol_table, id);
				}
			}
			
			if(!fn) {
				printf("error: call to undefined function \"%s\"\n", id);
				dump_fn_call_stack();
				exit(1);
			} else {

				como_stack_push_ex(&cg->global_call_stack, call_info_create(
					id, fcall_lineno, fcall_colno
				));
				
				Object* call_info = newArray(3);
				Object* fname = newString(cg->active_function_name);
				arrayPush(call_info, fname);
				objectDestroy(fname);
				Object* lineno = newLong(p->u1.call_node.lineno);
				Object* colno = newLong(p->u1.call_node.colno);
				arrayPush(call_info, lineno);
				arrayPush(call_info, colno);
				objectDestroy(lineno); 
				objectDestroy(colno);

				como_stack_push(&cg->function_call_stack, call_info);
		

				if(O_TYPE(fn) != IS_FUNCTION) {
					printf("error: \"%s\" is not callable\n", id);
					dump_fn_call_stack();
					return newNull();
				}

				if((O_MRKD(fn)) & C_USER_FUNC) {
					// actual arguments passed to function
					ast_node_statements* arg_list = &(p->u1.call_node.arguments->u1.statements_node);
					// function defintion
					ast_node_function* fn_info = &(((ast_node*)(O_FVAL(fn)))->u1.function_node);
					// declaration parameters
					ast_node_statements* arg_info = &(fn_info->parameter_list->u1.statements_node);

					if(arg_info->count != arg_list->count) {
						printf("error: function \"%s\" expects exactly %zu argument(s), but only %zu were passed\n", 
							id, arg_info->count, arg_list->count);
						dump_fn_call_stack();
						exit(1);
					}
						
					Object* old_current_symbol_table = cg->current_symbol_table;
					const char* prev_active_func_name = cg->active_function_name;
					Object* scope = newMap(2);
					cg->active_function_name = id;
					
					size_t i;
					for(i = 0; i < arg_info->count; i++) {
						ast_node* param = arg_info->statement_list[i];
						const char* param_name = param->u1.id_node.name;
						Object* val = ex(arg_list->statement_list[i]); 
						mapInsert(scope, param_name, val);
						objectDestroy(val);
					}

					cg->current_symbol_table = scope;

					Object* fn_ret_val;
					ast_node* body = fn_info->body;
					
					fn_ret_val = ex(body);

					if(cg->return_value == NULL) {
						cg->return_value = newNull();
					}

					cg->active_function_name = prev_active_func_name;
					objectDestroy(scope);
					cg->current_symbol_table = old_current_symbol_table;
					objectDestroy(fn);
					objectDestroy(fn_ret_val);

					Object* retval = copyObject(cg->return_value);	
					objectDestroy(cg->return_value);
					cg->return_value = NULL;
					return retval;	
				} else if(O_MRKD(fn) & C_EXT_FUNC) {
					// actual arguments passed to function
					ast_node_statements* arg_list = &(p->u1.call_node.arguments->u1.statements_node);
					// function defintion
					como_ext_func native_func = (como_ext_func)(O_FVAL(fn));

					Object* old_current_symbol_table = cg->current_symbol_table;
					const char* prev_active_func_name = cg->active_function_name;
					Object* scope = newMap(2);
					cg->active_function_name = id;
					Object* native_args = newArray(2);	
					size_t i;
					for(i = 0; i < arg_list->count; i++) {
						ast_node* param = arg_list->statement_list[i];
						const char* param_name = param->u1.id_node.name;
						Object* val = ex(arg_list->statement_list[i]); 
						mapInsert(scope, param_name, val);
						arrayPush(native_args, val);
						objectDestroy(val);
					}

					cg->current_symbol_table = scope;

					Object* fn_ret_val;

					native_func(native_args, &fn_ret_val);

					if(cg->return_value == NULL) {
						cg->return_value = newNull();
					}

					cg->active_function_name = prev_active_func_name;
					objectDestroy(scope);
					cg->current_symbol_table = old_current_symbol_table;
					objectDestroy(fn);
					objectDestroy(fn_ret_val);
					objectDestroy(native_args);
					Object* retval = copyObject(cg->return_value);	
					objectDestroy(cg->return_value);
					cg->return_value = NULL;
					return retval;	
				} else {
					return NULL;
				}
			}
		} break;
		case AST_NODE_TYPE_BIN_OP: {
			switch(p->u1.binary_node.type) {
				default:
					printf("%s(): invalid binary op(%d)\n", __func__, p->u1.binary_node.type);
					exit(1);
				break;
				case AST_BINARY_OP_NOT_EQUAL: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);

					if(!left || !right) {
						printf("ex: binary_op_not_equal: null\n");
						return NULL;
					} else {
						int equal = objectValueCompare(left, right);
						if(!equal) {
							return newLong(1);
						} else {
							return newLong(0);
						}
					}
				} break;
				case AST_BINARY_OP_LESS_THAN: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);
					if(!left || !right) {
						printf("ex: binary_op_less_than: null\n");
						return NULL;
					} else {
						int sametype = object_value_is_less_than(left, right);
						return newLong((long)sametype);
					}
				} break;
				case AST_BINARY_OP_GREATER_THAN: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);
					if(!left || !right) {
						printf("ex: binary_op_greater_than: null\n");
						return NULL;
					} else {
						int sametype = object_value_is_greater_than(left, right);
						return newLong((long)sametype);
					}
				} break;
				case AST_BINARY_OP_DOT: {
					printf("dot operator not implemented\n");
					exit(1);				
				} break;
				case AST_BINARY_OP_CMP: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);

					if(!left || !right) {
						return NULL;
					} else {
						return newLong(objectValueCompare(left, right));
					}
				}
				break;
				case AST_BINARY_OP_MINUS: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);

					if(!left || !right) {
						return NULL;
					} else {
						if(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG) {
							long diff = O_LVAL(left) - O_LVAL(right);
							O_LVAL(left) = diff;
							return left;
						} else {
							return newNull();
						}
					}
				}
				break;
				case AST_BINARY_OP_DIV: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);

					if(!left || !right) {
						return NULL;
					} else {
						if(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG) {
							if(!O_LVAL(right)) {
								printf("error: division by zero is undefined\n");
								dump_fn_call_stack();
								exit(1);
							}
							long result = O_LVAL(left) / O_LVAL(right);
							objectDestroy(left);
							objectDestroy(right);
							return newLong(result);	
						} else {
							return newNull();
						}
					}
				}
				break;
				case AST_BINARY_OP_ADD: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);
					if(!left || !right) {
						return NULL;
					} else {
						if(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG) {
							long sum = O_LVAL(left) + O_LVAL(right);
							objectDestroy(left);
							objectDestroy(right);
							return newLong(sum);	
						} else {
							char *leftstr = objectToString(left);
							char *rightstr = objectToString(right);
							Object *oleft = newString(leftstr);
							Object *oright = newString(rightstr);
							Object *ret = stringCat(oleft, oright);
							free(leftstr);
							free(rightstr);
							objectDestroy(oleft);
							objectDestroy(oright);
							return ret;
						}
					}
				}
				break;
				case AST_BINARY_OP_TIMES: {
					Object* left = ex(p->u1.binary_node.left);
					Object* right = ex(p->u1.binary_node.right);

					if(!left || !right) {
						return NULL;
					} else {
						if(O_TYPE(left) == IS_LONG && O_TYPE(right) == IS_LONG) {
							long sum = O_LVAL(left) * O_LVAL(right);
							objectDestroy(left); objectDestroy(right);
							return newLong(sum);	
						} else {
							return newNull();
						}
					}
				} break;
				case AST_BINARY_OP_ASSIGN: {
					const char* id = p->u1.binary_node.left->u1.id_node.name;
					Object* right = ex(p->u1.binary_node.right);					
					if(cg->current_symbol_table == NULL) {
						mapInsert(cg->symbol_table, id, right);
					} else {
						mapInsert(cg->current_symbol_table, id, right);
					}
					
					Object* ret = copyObject(right);
					objectDestroy(right);

					return ret;
				} break;
			}	
		} break;
	}
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
	
	Object* ret = ex(program);
	
	OBJECT_DUMP(cg->symbol_table);

	objectDestroy(cg->symbol_table);
	objectDestroy(ret);
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
