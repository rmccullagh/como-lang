#ifndef MODULE_H
#define MODULE_H

#include <object.h>

#define COMO_FUNC_NAME(n) ci_##n
#define COMO_FUNC_ARGS Object** retval
#define COMO_FUNC(name) void COMO_FUNC_NAME(name)(COMO_FUNC_ARGS)

typedef struct como_function como_function;

typedef struct {
	const char* name;
	const como_function* functions;
} como_module;

struct como_function {
	const char* name;
	void(*handler)(COMO_FUNC_ARGS);
};


#endif
