#ifndef COMO_EXECUTE_H
#define COMO_EXECUTE_H

#include <object.h>

#include "como_compiler.h"
#include "stack.h"

typedef struct como_executor_globals {
	Object        *filename;
	como_stack    *scope;   
	como_op_array *op_array;
} como_executor_globals;

como_executor_globals *como_executor_globals_create(const char *, como_op_array *);

#endif