#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <object.h>

#include "ast.h"
#include "como_execute.h"
#include "stack.h"
#include "como_compiler.h"
#include "comodebug.h"
#include "como_opcode.h"

como_executor_globals *ceg;

void ast_compile(const char *filename, ast_node* program)
{
	assert(program != NULL);
	
	como_op_array *op_array = como_op_array_create();

	if((ceg = como_executor_globals_create(filename, op_array)) == NULL) {
		como_error_noreturn("como_executor_globals_created failed");
	}

	Object *call_info = newArray(3);
	arrayPushEx(call_info, newString("__main__"));
	arrayPushEx(call_info, newLong(0L));
	arrayPushEx(call_info, newLong(0L));

	como_stack_push(&CG(op_array)->call_stack, (void *)call_info);


}
