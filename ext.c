#include "ext.h"
#include "module.h"

static const como_function standard_functions[] = {
	{"swap", COMO_FUNC_NAME(swap) },
	{NULL, NULL}	
};

como_module standard_module = {
	"standard",
	standard_functions
};

COMO_FUNC(swap)
{
	if(como_parse_args("%s", &str) == None) {
		return;
	}

	RETURN_TRUE();
}



