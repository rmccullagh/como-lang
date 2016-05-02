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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <object.h>
#include <assert.h>
#include "ast.h"
#include "stack.h"
#include "globals.h"
#include "object_api.h"
#include "comodebug.h"
#include "comotypes.h"
#include "callframe.h"

typedef struct compiler_context {
	Object* symbol_table;
	Object* context;
	como_object *current_object;
	como_object *retval;
} compiler_context;

static compiler_context* cg;

static como_object *
como_builtin_print(como_object *self, Object *args)
{
	if(self == NULL) {
		como_error_noreturn("self is NULL\n");
	}
	
	size_t i;

	Array *_args = O_AVAL(args);

	for(i = 0; i < _args->size; i++) {
		Object *value = _args->table[i];
		OBJECT_DUMP(value);
	}

	return como_type_new_int_object(1);
}

static void setup_builtin(Object *symtable, const char *name, void *impl)
{
	Object *builtin_optr = newFunction(impl);
	como_object *container = como_type_new_function_object(NULL, builtin_optr);
	container->self = container;
	container->flags |= COMO_TYPE_IS_SEALED;
	container->flags |= COMO_TYPE_IS_BUILTIN;
	
	Object *fobj = newPointer((void *)container);
	O_MRKD(fobj) = COMO_TYPE_IS_OBJECT;
	
	mapInsert(symtable, name, fobj);

}

static void compiler_init(void)
{
	cg = malloc(sizeof(compiler_context));
	cg->symbol_table = newMap(2);
	cg->context = NULL;
	cg->retval = NULL;
	cg->current_object = NULL;
	setup_builtin(cg->symbol_table, "print", como_builtin_print);
}

static como_object *ex(ast_node *);
static como_object *como_do_call(ast_node *);

static Object *validate_call_args(ast_node *n)
{
	if(n->type != AST_NODE_TYPE_STATEMENT_LIST) {
		como_error_noreturn("n.type is not AST_NODE_TYPE_STATEMENT_LIST\n");
	}
	Object *arglist = newArray(2);
	size_t i;
	ast_node_statements slist = n->u1.statements_node;
	for(i = 0; i < slist.count; i++) {
		ast_node* stmt = slist.statement_list[i];
		if(stmt->type == AST_NODE_TYPE_STATEMENT_LIST) {
			como_error_noreturn("argument %zu cannot be a statement list\n", i);
		}
		como_object *evaluated = ex(stmt);
		arrayPush(arglist, evaluated->value);
	}
	return arglist;
}

static como_object *como_do_call(ast_node *p)
{
	como_object *callablevar = ex(p->u1.call_node.expression);
	como_object *retval = NULL;

	if(!(callablevar->flags & COMO_TYPE_IS_CALLABLE)) {
		como_error_noreturn("value of type '%s' is not callable (%d:%d)\n", 
				callablevar->type->name, p->u1.call_node.expression->lineno, 
				p->u1.call_node.expression->colno); 
	}
	if(callablevar->flags & COMO_TYPE_IS_BUILTIN) {
		Object *fimpl = callablevar->value;
		if(O_TYPE(fimpl) != IS_FUNCTION) {
			como_error_noreturn("object container type was not IS_FUNCTION\n");
		}
		Object *call_args = validate_call_args(p->u1.call_node.arguments);
		como_type_method method = (como_type_method)O_FVAL(fimpl);
		retval = method(callablevar->self, call_args);
	} else {
		Object *fimpl = callablevar->value;
		if(O_TYPE(fimpl) != IS_POINTER) {
			como_error_noreturn("object container type was not IS_POINTER\n");
		}
		cg->current_object = callablevar;
		ast_node *fn = (ast_node *)O_PTVAL(fimpl);
		Object *fnsymtab = newMap(2);
		if(fn->type == AST_NODE_TYPE_FUNC_DEFN) {
			size_t arg_count = p->u1.call_node.arguments->u1.statements_node.count;
			size_t param_count = fn->u1.function_defn_node.parameters->u1.statements_node.count;
			size_t i;
			if(arg_count != param_count) {
				como_error_noreturn("function '%s' expects %zu arguments, but only %zu given (%d:%d)\n",
						fn->u1.function_defn_node.name->u1.id_node.name, param_count, arg_count,
						p->lineno, p->colno);
			}
		
			ast_node_statements parameters = fn->u1.function_defn_node.parameters->u1.statements_node;
			ast_node_statements arguments = p->u1.call_node.arguments->u1.statements_node;

			for(i = 0; i < param_count; i++) {
				const char *param_name = parameters.statement_list[i]->u1.id_node.name;
				como_object *argvalue = ex(arguments.statement_list[i]);
				if(!argvalue) {
					como_error_noreturn("argument %zu was null\n", i);
				}
				Object *paramval = newPointer((void *)argvalue);
				O_MRKD(paramval) = COMO_TYPE_IS_OBJECT;
				mapInsert(fnsymtab, param_name, paramval); 
			}

			cg->context = fnsymtab;
			assert(fn->u1.function_defn_node.body->type == AST_NODE_TYPE_STATEMENT_LIST);

			ast_node_statements fstatements = fn->u1.function_defn_node.body->u1.statements_node;
			for(i = 0; i < fstatements.count; i++) {
				ast_node *st = fstatements.statement_list[i];
				if(st->type == AST_NODE_TYPE_RETURN) {
					cg->retval = ex(st);
					break;
				}	else {
					(void)ex(st);
				}
			}
		} else {
			size_t arg_count = p->u1.call_node.arguments->u1.statements_node.count;
			size_t param_count = fn->u1.anon_func_node.parameters->u1.statements_node.count;
			size_t i;
			if(arg_count != param_count) {
				como_error_noreturn("function '%s' expects %zu arguments, but only %zu given (%d:%d)\n",
						"<anonymous>", param_count, arg_count,
						p->lineno, p->colno);
			}
		
			ast_node_statements parameters = fn->u1.anon_func_node.parameters->u1.statements_node;
			ast_node_statements arguments = p->u1.call_node.arguments->u1.statements_node;

			for(i = 0; i < param_count; i++) {
				const char *param_name = parameters.statement_list[i]->u1.id_node.name;
				como_object *argvalue = ex(arguments.statement_list[i]);
				if(!argvalue) {
					como_error_noreturn("argument %zu was null\n", i);
				}
				Object *paramval = newPointer((void *)argvalue);
				O_MRKD(paramval) = COMO_TYPE_IS_OBJECT;
				mapInsert(fnsymtab, param_name, paramval); 
			}

			cg->context = fnsymtab;
			assert(fn->u1.anon_func_node.body->type == AST_NODE_TYPE_STATEMENT_LIST);

			ast_node_statements fstatements = fn->u1.anon_func_node.body->u1.statements_node;
			for(i = 0; i < fstatements.count; i++) {
				ast_node *st = fstatements.statement_list[i];
				if(st->type == AST_NODE_TYPE_RETURN) {
					cg->retval = ex(st);
					break;
				}	else {
					(void)ex(st);
				}
			}
		}

		retval = cg->retval == NULL ? como_type_new_undefined_object() : cg->retval;
		cg->context = NULL;
		objectDestroy(fnsymtab);
		cg->retval = NULL;
		cg->current_object = NULL;
	}
	return retval;
}

static como_object *como_do_func_defn(ast_node *p, int anon)
{
	assert(p->type == AST_NODE_TYPE_FUNC_DEFN);
	ast_node_function_defn defn = p->u1.function_defn_node;
	assert(defn.name->type == AST_NODE_TYPE_ID);
	const char *name = defn.name->u1.id_node.name;
	ast_node_statements *body = &defn.body->u1.statements_node;
	size_t i;
	for(i = 0; i < body->count; i++) {
		if(body->statement_list[i]->type == AST_NODE_TYPE_FUNC_DEFN) {
			como_error_noreturn("nested function definitions are forbidden (%d:%d)\n",
					body->statement_list[i]->lineno, body->statement_list[i]->colno);
		}
	}
	Object *builtin_optr = newPointer((void *)p);
	como_object *container = como_type_new_function_object(NULL, builtin_optr);
	container->self = container;
	if(!anon) {
		Object *fobj = newPointer((void *)container);
		O_MRKD(fobj) = COMO_TYPE_IS_OBJECT;
		mapInsert(cg->symbol_table, name, fobj);
	}
	return container;
}

static como_object *como_create_anon_func(ast_node *p)
{
	assert(p->type == AST_NODE_TYPE_ANON_FUNC);
	ast_node_anon_func_defn defn = p->u1.anon_func_node;
	ast_node_statements *body = &defn.body->u1.statements_node;
	size_t i;
	for(i = 0; i < body->count; i++) {
		if(body->statement_list[i]->type == AST_NODE_TYPE_FUNC_DEFN) {
			como_error_noreturn("nested function definitions are forbidden (%d:%d)\n",
					body->statement_list[i]->lineno, body->statement_list[i]->colno);
		}
	}
	Object *builtin_optr = newPointer((void *)p);
	como_object *container = como_type_new_function_object(NULL, builtin_optr);
	container->self = container;
	return container;
}

static como_object *como_do_create_instance(ast_node *p)
{
	assert(p->type == AST_NODE_TYPE_NEW);
	ast_node *expr = p->u1.new_node.expression;
	assert(expr->type == AST_NODE_TYPE_CALL);
	return ex(expr);
}

static como_object* ex(ast_node* p)
{
	if(!p)
		return NULL;

	switch(p->type) {
		default:
			como_error_noreturn("invalid ast.node_type\n");
		break;
		case AST_NODE_TYPE_NEW:
			return como_do_create_instance(p);
		break;
		case AST_NODE_TYPE_RETURN:
			return ex(p->u1.return_node.expression);
		break;
		case AST_NODE_TYPE_FUNC_DEFN:
			return como_do_func_defn(p, 0);
		break;
		case AST_NODE_TYPE_STRING:
			return como_type_new_string_object(p->u1.string_value.value);
		break;
		case AST_NODE_TYPE_NUMBER:
			return como_type_new_int_object(p->u1.number_value);
		break;
		case AST_NODE_TYPE_DOUBLE:
			return como_type_new_double_object(p->u1.double_value);
		break;
		case AST_NODE_TYPE_CALL: {
			return como_do_call(p);
		}
		break;
		case AST_NODE_TYPE_ID: {
			Object* value = NULL;
			if(cg->context == NULL) {
				value = mapSearchEx(cg->symbol_table, p->u1.id_node.name);
				if(!value) {
					como_error_noreturn("'%s' is not defined (%d:%d)\n", 
							p->u1.id_node.name, p->lineno, p->colno); 
				} else {
					if(O_TYPE(value) != IS_POINTER) {
						como_error_noreturn("O_TYPE: value of '%s' is not IS_FUNCTION\n",
								p->u1.id_node.name);
					}
					if(O_MRKD(value) & COMO_TYPE_IS_OBJECT) {
						return (como_object *)O_PTVAL(value);
					} else {
						como_error_noreturn("O_MRKD(value) is not & COMO_TYPE_IS_OBJECT\n");
					}
				}
			} else {
				if(strcmp(p->u1.id_node.name, "this") == 0) {
					return cg->current_object;
				}
				value = mapSearchEx(cg->context, p->u1.id_node.name);
				if(!value) {
					value = mapSearchEx(cg->symbol_table, p->u1.id_node.name);
				}
				if(!value) {
					como_error_noreturn("'%s' is not defined (%d:%d)\n", 
							p->u1.id_node.name, p->lineno, p->colno); 
				} else {
					if(O_TYPE(value) != IS_POINTER) {
						como_error_noreturn("O_TYPE: value of '%s' is not IS_FUNCTION\n",
								p->u1.id_node.name);
					}
					if(O_MRKD(value) & COMO_TYPE_IS_OBJECT) {
						return (como_object *)O_PTVAL(value);
					} else {
						como_error_noreturn("O_MRKD(value) is not & COMO_TYPE_IS_OBJECT\n");
					}
				}
			}
			return NULL;
		}
		break;
		case AST_NODE_TYPE_STATEMENT_LIST: {
			size_t i;
			for(i = 0; i < p->u1.statements_node.count; i++) {
				ast_node* stmt = p->u1.statements_node.statement_list[i];
				(void )ex(stmt);
			}
			return como_type_new_undefined_object();
		} break;
		case AST_NODE_TYPE_BIN_OP: {
			switch(p->u1.binary_node.type) {
				default:
					como_error_noreturn("invalid AST_NODE_TYPE_BIN_OP\n");
				break;
				case AST_BINARY_OP_DOT: {
					/* this */
					como_object *parent = ex(p->u1.binary_node.left);
					como_debug("Type of the primary in the dot operation: %s\n", parent->type->name);
					if(O_TYPE(parent->value) == IS_NULL) {
							como_error_noreturn("property is not defined (%d:%d)\n",
									p->u1.binary_node.left->lineno, p->u1.binary_node.left->colno);
					}
					if(p->u1.binary_node.right->type != AST_NODE_TYPE_ID) {
						como_error_noreturn("binary_node.right.type != AST_NODE_TYPE_ID\n");
					}

					const char *key = p->u1.binary_node.right->u1.id_node.name;
					Object *keyvalue = mapSearchEx(parent->type->properties,
																key);
					if(keyvalue == NULL) {
						return como_type_new_undefined_object();
					}
					if(O_TYPE(keyvalue) != IS_POINTER) {
						como_error_noreturn("'%s' is not a IS_FUNCTION (%d)\n", 
								key, p->u1.binary_node.right->lineno);
					}
					if(O_MRKD(keyvalue) & COMO_TYPE_IS_OBJECT) {
						return (como_object *)O_PTVAL(keyvalue);
					} else {
						como_error_noreturn("identifier '%s' type is not implicit COMO_TYPE_IS_OBJECT (%d)\n", 
								key, p->u1.binary_node.right->lineno);
					}
				} break;
				case AST_BINARY_OP_ADD: {
					como_object* left  = ex(p->u1.binary_node.left);
					como_object* right = ex(p->u1.binary_node.right);
					if(O_TYPE(left->value) == IS_LONG && O_TYPE(right->value) == IS_LONG) {
						long sum = O_LVAL(left->value) + O_LVAL(right->value);
						return como_type_new_int_object(sum);	
					} else {
						como_error_noreturn("AST_BINARY_OP_ADD: only works on longs\n");
					}
				}
				break;
				case AST_BINARY_OP_ASSIGN: {
					if(p->u1.binary_node.left->type == AST_NODE_TYPE_ID) {
						const char *left = p->u1.binary_node.left->u1.id_node.name;
						Object *v;
						if(cg->context == NULL) {
							v = mapSearch(cg->symbol_table, left);
						} else {
							v = mapSearch(cg->context, left);
							if(v == NULL) {
								v = mapSearch(cg->symbol_table, left);
							}
						}
						if(v != NULL) {
							if(O_TYPE(v) == IS_POINTER) {
								if(O_MRKD(v) & COMO_TYPE_IS_OBJECT) {
									como_object *ov = (como_object *)O_PTVAL(v);
									if(ov->flags & COMO_TYPE_IS_SEALED) {
										como_error_noreturn("can't assign to read-only identifier '%s' (%d:%d)\n", 
												left, p->lineno, p->colno);	
									}
								}
							}
						}
						como_object *right;
						if(p->u1.binary_node.right->type == AST_NODE_TYPE_ANON_FUNC) {
							right = como_create_anon_func(p->u1.binary_node.right);
						} else {
							right = ex(p->u1.binary_node.right);
						}
						Object *value = newPointer((void *)right);
						O_MRKD(value) = COMO_TYPE_IS_OBJECT;
						if(cg->context == NULL) {
							mapInsertEx(cg->symbol_table, left, value);
						} else {
							mapInsertEx(cg->context, left, value);
						}
						return right;
					} else {
						como_object *primary = ex(p->u1.binary_node.left->u1.binary_node.left);
						const char *id = "<unknown>";
						if(p->u1.binary_node.left->u1.binary_node.left->type == AST_NODE_TYPE_ID) {
							id = p->u1.binary_node.left->u1.binary_node.left->u1.id_node.name;
						}
						if(O_TYPE(primary->value) == IS_NULL) {
							como_error_noreturn("property '%s' is not defined (%d)\n",
									id, p->u1.binary_node.left->u1.binary_node.left->lineno);
						}
						const char *name = p->u1.binary_node.left->u1.binary_node.right->u1.id_node.name;
						Object *v = mapSearch(primary->type->properties, name);
						if(v != NULL) {
							if(O_TYPE(v) == IS_POINTER) {
								if(O_MRKD(v) & COMO_TYPE_IS_OBJECT) {
									como_object *ov = (como_object *)O_PTVAL(v);
									if(ov->flags & COMO_TYPE_IS_SEALED) {
										como_error_noreturn("property '%s' is read-only\n", name);	
									}
								}
							}
						}
						como_object *right;
						if(p->u1.binary_node.right->type == AST_NODE_TYPE_ANON_FUNC) {
							right = como_create_anon_func(p->u1.binary_node.right);
							Object *value = newPointer((void *)right);
							O_MRKD(value) = COMO_TYPE_IS_OBJECT;
							mapInsert(primary->type->properties, name, value);
						} else {
							right = ex(p->u1.binary_node.right);
							Object *value = newPointer((void *)right);
							O_MRKD(value) = COMO_TYPE_IS_OBJECT;
							mapInsert(primary->type->properties, name, value);
						}
						return right;
					}	
				} break;
			}	
		} break;
	}
	return NULL;
}

void ast_compile(const char* filename, ast_node* program, int show_sym)
{
	if(!program) {
		printf("%s(): unexpected empty node\n", __func__);
		exit(1);
	}

	compiler_init();

	(void)ex(program);	

	if(show_sym) {	
		Map *symtab = O_MVAL(cg->symbol_table);
		
		uint32_t i;

		for(i = 0; i < symtab->capacity; i++) {
			if(symtab->buckets[i] != NULL) {
				Bucket *b = symtab->buckets[i];
				while(b != NULL) {
					Bucket *next = b->next;
						printf("%s :", b->key->value);
						Object *value = b->value;
						if(O_TYPE(value) == IS_POINTER) {
							como_object *ob = (como_object *)O_PTVAL(value);
							if(ob) {
								fprintf(stdout, "<%p>: ", (void *)ob->value);
								OBJECT_DUMP(ob->value);
								OBJECT_DUMP(ob->type->properties);
							}
						}
					b = next;
				}
			}
		}
	}
}
