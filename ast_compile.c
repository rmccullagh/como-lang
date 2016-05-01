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
#include "comodebug.h"
#include "comotypes.h"
#include "callframe.h"

typedef struct compiler_context {
	Object* symbol_table;
	como_object *error_object;	
} compiler_context;

static compiler_context* cg;

static como_object *
como_builtin_typeof(como_object *self, Object *args)
{
	if(self == NULL) {
		como_error_noreturn("self is NULL\n");
	}
	return como_type_init_string(self->type->name);		
}

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
	Object *builtin_typeof_cfunc = newFunction(impl);
	O_MRKD(builtin_typeof_cfunc) = COMO_TYPE_IS_FUNC;

	como_object *container = como_type_new_function_object(NULL, 
			builtin_typeof_cfunc);

	container->self = container;
	Object *fobj = newFunction((void *)container);
	O_MRKD(fobj) = COMO_TYPE_IS_OBJECT;
	
	mapInsert(symtable, name, fobj);

}

static void compiler_init(void)
{
	cg = malloc(sizeof(compiler_context));
	cg->symbol_table = newMap(2);
	cg->error_object = NULL;

	Object *builtin_typeof_cfunc = newFunction(como_builtin_typeof);
	O_MRKD(builtin_typeof_cfunc) = COMO_TYPE_IS_FUNC;

	como_object *container = como_type_new_function_object(NULL, 
			builtin_typeof_cfunc);
	container->self = container;
	Object *fobj = newFunction((void *)container);
	O_MRKD(fobj) = COMO_TYPE_IS_OBJECT;
	mapInsert(cg->symbol_table, "typeof", fobj);

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
	if(p == NULL) {
		como_error_noreturn("ast_node is NULL\n");
	}
	
	como_object *callablevar = ex(p->u1.call_node.expression);

	if(!callablevar) {
		como_error_noreturn("ex returned NULL\n");
	}
	
	if(!(callablevar->flags & COMO_TYPE_IS_CALLABLE)) {
		como_error_noreturn("value of type '%s' is not callable (%d:%d)\n", 
				callablevar->type->name, p->u1.call_node.expression->lineno, 
				p->u1.call_node.expression->colno); 
	}

	Object *fimpl = callablevar->value;

	if(O_TYPE(fimpl) != IS_FUNCTION) {
		como_error_noreturn("object container type was not IS_FUNCTION\n");
	}

	if(!(O_MRKD(fimpl) & COMO_TYPE_IS_FUNC)) {
		como_error_noreturn("object implicit type is not COMO_TYPE_IS_FUNC\n");	
	}

	Object *call_args = validate_call_args(p->u1.call_node.arguments);

	como_type_method method = (como_type_method)O_FVAL(fimpl);

	if(callablevar->self == NULL) {
		como_error_noreturn("self was NULL\n");
	}

	return method(callablevar->self, call_args);
}

static como_object* ex(ast_node* p)
{
	if(!p)
		return NULL;

	switch(p->type) {
		default:
			como_error_noreturn("invalid ast.node_type\n");
		break;
		case AST_NODE_TYPE_STRING:
			return como_type_init_string(p->u1.string_value.value);
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
			Object* value;
			value = mapSearchEx(cg->symbol_table, p->u1.id_node.name);
			if(!value) {
				como_error_noreturn("'%s' is not defined (%d:%d)\n", 
						p->u1.id_node.name, p->lineno, p->colno); 
			} else {
				if(O_TYPE(value) != IS_FUNCTION) {
					como_error_noreturn("O_TYPE: value of '%s' is not IS_FUNCTION\n",
							p->u1.id_node.name);
				}
				if(O_MRKD(value) & COMO_TYPE_IS_OBJECT) {
					return (como_object *)O_FVAL(value);
				} else {
					como_error_noreturn("O_MRKD(value) is not & COMO_TYPE_IS_OBJECT\n");
				}
			}
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
					if(O_TYPE(keyvalue) != IS_FUNCTION) {
						como_error_noreturn("'%s' is not a IS_FUNCTION (%d)\n", 
								key, p->u1.binary_node.right->lineno);
					}
					if(O_MRKD(keyvalue) & COMO_TYPE_IS_OBJECT) {
						return (como_object *)O_FVAL(keyvalue);
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
						como_object *right = ex(p->u1.binary_node.right);
						Object *value = newFunction((void *)right);
						O_MRKD(value) = COMO_TYPE_IS_OBJECT;
						mapInsertEx(cg->symbol_table, left, value);
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
							if(O_TYPE(v) == IS_FUNCTION) {
								if(O_MRKD(v) & COMO_TYPE_IS_SEALED) {
									como_error_noreturn("property '%s' is read-only\n", name);	
								}
							}
						}

						como_object *right = ex(p->u1.binary_node.right);
						Object *value = newFunction((void *)right);
						O_MRKD(value) = COMO_TYPE_IS_OBJECT;
						mapInsert(primary->type->properties, name, value);
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
						printf("%s:", b->key->value);
						Object *value = b->value;
						if(O_MRKD(value) & COMO_TYPE_IS_OBJECT) {
							como_object *ob = (como_object *)O_FVAL(value);
							if(ob) {
								fprintf(stdout, "<%p>: ", (void *)ob->value);
								OBJECT_DUMP(ob->value);
								fputc('\n', stdout);
							}
						}
					b = next;
				}
			}
		}
	}
}
