#include <object.h>

#include "ext.h"

COMO_FUNC(swap)
{
	OBJECT_DUMP(args);

	*retval = NULL;
}

static const como_function standard_functions[] = {
	{"swap", COMO_FUNC_NAME(swap) },
	{NULL, NULL}	
};

como_module standard_module = {
	"standard",
	standard_functions
};

como_module* module_init(void)
{
	return &standard_module;
}


