#ifndef COMO_GLOBALS_H
#define COMO_GLOBALS_H

#include <stddef.h>

typedef struct como_global {
	char* filename;
	size_t filename_length;
} como_global;

extern const char* get_file_name();

extern void dump_fn_call_stack();

#endif
