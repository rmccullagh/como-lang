#ifndef MODULE_H
#define MODULE_H

#include <object.h>

#define COMO_FUNC_NAME(n) como_##n
#define COMO_FUNC_ARGS Object* args, Object** retval
#define COMO_FUNC(name) void COMO_FUNC_NAME(name)(COMO_FUNC_ARGS)

typedef void(*como_native_func)(COMO_FUNC_ARGS);

typedef struct como_function como_function;

typedef struct {
	const char* name;
	const como_function* functions;
} como_module;

struct como_function {
	const char* name;
	void(*handler)(COMO_FUNC_ARGS);
};

typedef struct como_module_list {
	void* handle;
	como_module* module_info;
} como_module_list;

#endif
