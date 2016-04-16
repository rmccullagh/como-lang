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

typedef struct compiler_context {
	Object* symbol_table;
} compiler_context;

static compiler_context* cg;

static void compiler_init(void)
{
	cg = malloc(sizeof(compiler_context));
	cg->symbol_table = newMap(2);
}

static como_object *ex(ast_node *);
static como_object *como_do_call(ast_node *);

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
		como_error_noreturn("'%s' object is not callable\n", 
				callablevar->type->name); 
	}

	DEBUG_TYPE(callablevar->value);
	
	Object *fimpl = callablevar->value;

	if(O_TYPE(fimpl) != IS_FUNCTION) {
		como_error_noreturn("object container type was not IS_FUNCTION\n");
	}

	if(!(O_MRKD(fimpl) & COMO_TYPE_IS_FUNC)) {
		como_error_noreturn("object implicit type is not COMO_TYPE_IS_FUNC\n");	
	}

	como_type_method method = (como_type_method)O_FVAL(fimpl);

	return method(callablevar->self, NULL);

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
			value = mapSearch(cg->symbol_table, p->u1.id_node.name);
			if(!value) {
				printf("warning: undefined variable %s\n", p->u1.id_node.name);
				return como_type_new_int_object(0);
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
			return como_type_new_int_object(0);
		} break;
		case AST_NODE_TYPE_BIN_OP: {
			switch(p->u1.binary_node.type) {
				default:
					como_error_noreturn("invalid AST_NODE_TYPE_BIN_OP\n");
				break;
				case AST_BINARY_OP_DOT: {
					/* this */
					como_object *parent = ex(p->u1.binary_node.left);
					if(p->u1.binary_node.right->type != AST_NODE_TYPE_ID) {
						como_error_noreturn("binary_node.right.type != AST_NODE_TYPE_ID\n");
					}
					const char *key = p->u1.binary_node.right->u1.id_node.name;
					Object *keyvalue = mapSearchEx(parent->type->properties,
																key);
					if(keyvalue == NULL) {
						como_error_noreturn("warning: undefined property '%s'\n", key);
					}
					if(O_TYPE(keyvalue) != IS_FUNCTION) {
						como_error_noreturn("como: warning: not function property '%s'\n", key);
					}
					if(O_MRKD(keyvalue) & COMO_TYPE_IS_FUNC) {
						return como_type_new_function_object(parent, keyvalue);
					} else {
						como_error_noreturn("unknown  property type '%s'\n", key);
					}
				} break;
				case AST_BINARY_OP_ADD: {
					como_object* left  = ex(p->u1.binary_node.left);
					como_object* right = ex(p->u1.binary_node.right);
					if(O_TYPE(left->value) == IS_LONG && O_TYPE(right->value) == IS_LONG) {
						long sum = O_LVAL(left->value) + O_LVAL(right->value);
						return como_type_new_int_object(sum);	
					} else {
						como_error_noreturn("AST_BINARY_OP_ADD: only workds on longs\n");
					}
				}
				break;
				case AST_BINARY_OP_ASSIGN: {
					const char* id = p->u1.binary_node.left->u1.id_node.name;
					como_object* right = ex(p->u1.binary_node.right);
					Object *value = newFunction((void *)right);
					O_MRKD(value) = COMO_TYPE_IS_OBJECT;
					Object *oval = mapSearch(cg->symbol_table, id);
					if(oval != NULL) {
						if(O_TYPE(oval) != IS_FUNCTION) {
							como_error_noreturn("object is IS_FUNCTION\n");
						}
						if(O_MRKD(oval) & COMO_TYPE_IS_OBJECT) {
							como_object *impl = (como_object *)O_FVAL(oval);
							if(impl->flags & COMO_TYPE_IS_SEALED) {
								como_error_noreturn("%s is sealed and cannot change\n", id);
							}
						}
					}	
					mapInsert(cg->symbol_table, id, value);
					objectDestroy(value);
					return right;
				} break;
			}	
		} break;
	}

	return NULL;
}

void ast_compile(const char* filename, ast_node* program)
{
	if(!program) {
		printf("%s(): unexpected empty node\n", __func__);
		exit(1);
	}

	compiler_init();

	(void)ex(program);	
	
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
							OBJECT_DUMP(ob->value);
						}
					}
				b = next;
			}
		}
	}
}
