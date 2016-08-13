#include <stdlib.h>
#include <object.h>

#include "como_execute.h"
#include "como_compiler.h"
#include "comodebug.h"

como_executor_globals *como_executor_globals_create(const char *filename, 
	como_op_array *op_array)
{
	como_executor_globals *ceg = malloc(sizeof(como_executor_globals));

	if(ceg == NULL) {
		return NULL;
	}

	ceg->filename = newString(filename);
	ceg->scope = NULL;
	if(ceg->filename == NULL) {
		free(ceg);
		return NULL;
	}

	ceg->op_array = op_array;

	return ceg;
}