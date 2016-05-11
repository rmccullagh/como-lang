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
#include <string.h>
#include <assert.h>
#include <object.h>
#include "ast.h"
#include "stack.h"
#include "globals.h"
#include "object_api.h"
#include "comodebug.h"

typedef struct compiler_context {
	Object *filename;
	Object *symbol_table;
} compiler_context;

static compiler_context* cg = NULL;
static Object* ex(ast_node *);

static int compiler_init(const char *filename)
{
	cg = malloc(sizeof(compiler_context));

	if(cg == NULL)
		return 1;

	cg->symbol_table = newMap(2);

	if(cg->symbol_table == NULL) {
		free(cg);
		cg = NULL;
		return 1;
	}

	cg->filename = newString(filename);

	if(cg->filename == NULL) {
		objectDestroy(cg->symbol_table);
		free(cg);
		cg = NULL;
		return 1;
	}

	return 0;
}

#define TARGET(t) case AST_NODE_TYPE_##t:

#define DEFAULT_TARGET default:

#define BIN_TARGET(t) case AST_BINARY_OP_##t:

static Object* ex(ast_node* p)
{
	if(!p)
		return NULL;

	switch(p->type) {
		DEFAULT_TARGET {
			como_error_noreturn("Invalid node type given\n");
		}
		TARGET(ASSIGN) {
			assert(p->u1.assign_node.name->type == AST_NODE_TYPE_ID);
			const char *name = p->u1.assign_node.name->u1.id_node.name;
			Object *value = ex(p->u1.assign_node.expression);
			mapInsertEx(cg->symbol_table, name, value);
			return value;
		}
		TARGET(STRING) {
			return newString(p->u1.string_value.value);
		}
		TARGET(NUMBER) {
			return newLong(p->u1.number_value);
		}
		TARGET(DOUBLE) {
			return newDouble(p->u1.double_value);
		}
		TARGET(ID) {
			const char *name = p->u1.id_node.name;
			Object *value = mapSearchEx(cg->symbol_table, name);
			if(value == NULL) {
				como_error_noreturn("Undefined variable %s\n", name);
			}
			return copyObject(value);
		}
		TARGET(STATEMENT_LIST) {
			size_t count = p->u1.statements_node.count;
			size_t i;
			ast_node_statements **statements = p->u1.statements_node.statement_list;
			Object *retval = NULL;
			for(i = 0; i < count; i++) {
				retval = ex(statements[i]);
			}
			return retval;
		} 
		TARGET(IF) {
			Object *condition = ex(p->u1.if_node.expression);
			if(object_is_truthy(condition)) {
				(void)ex(p->u1.if_node.b1);					
			} else {
				if(p->u1.if_node.b2 != NULL) {
					(void)ex(p->u1.if_node.b2);		
				}			
			}
			return NULL;
		} 
		TARGET(CALL) {
			const char *fname = p->u1.call_node.id->u1.id_node.name;
			if(strcmp(fname, "print") != 0) {
				como_error_noreturn("only print is a function\n");
			}
			size_t count = p->u1.statements_node.count;
			size_t i;
			ast_node_statements **statements = p->u1.statements_node.statement_list;
			Object *retval = NULL;
			for(i = 0; i < count; i++) {
				retval = ex(statements[i]);
				OBJECT_DUMP(retval);
			}
			return retval;
		}
		TARGET(BIN_OP) {
			DEFAULT_TARGET {
				como_error_noreturn("Invalid binary operation\n");
			}
			BIN_TARGET(ADD) {
				
			}
		}
	}
}

void ast_compile(const char* filename, ast_node* program)
{
	if(compiler_init(filename)) {
		como_error_noreturn("compiler_init failed\n");
		return;
	}
		
	(void)ex(program);	

}
