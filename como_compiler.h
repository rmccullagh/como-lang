#ifndef COMO_COMPILER_H
#define COMO_COMPILER_H

#include <stddef.h>
#include <object.h>

#include "stack.h"

#define COMO_DEFAULT_FRAME_STACKSIZE   2048U
#define COMO_DEFAULT_OP_ARRAY_CAPACITY 512U

#define COMO_NODE_OP_IS_LONG       0x01
#define COMO_NODE_OP_IS_DOUBLE     0x02
#define COMO_NODE_OP_IS_JMP_OFFSET 0x03
#define COMO_NODE_OP_IS_STRING     0x04
#define COMO_SUCCESS 0
#define COMO_FAILURE 1

typedef struct como_frame como_frame;

struct como_frame {
	size_t     cf_sp;                      /* stack pointer into cf_stack */
	size_t     cf_stack_size;              /* stack size, num of used entries */
	Object     *cf_fname;                  /* currently executing function name */
	Object     *cf_stack[(size_t)COMO_DEFAULT_FRAME_STACKSIZE];  /* stack */
	como_stack *cf_scope;               /* nested scopes for variables */
	Object     *cf_symtab;               /* Map, symbol table */
	Object     *cf_current_symtab;
	Object     *cf_retval;               /* return value */
};

typedef struct como_op_code {
	unsigned char op_code;
	Object       *operand;
} como_op_code;

typedef struct como_op_array {
	como_stack      *call_stack;
	como_frame      *frame;
	size_t           size;     /* size of the table */
	size_t           capacity; /* capacity of the table */
	size_t           pc;
	como_op_code   **table;    /* the array of instructions */
} como_op_array;

typedef struct como_function {
	Object        *name;
	Object        *arguments;
	como_op_array *op_array;
} como_function;

como_frame      *como_frame_create(void); 
void             como_frame_free(como_frame *);
como_op_array   *como_op_array_create(void);
void             como_emit(como_op_array*, unsigned char, Object *);

#endif /* !COMO_COMPILER_H */