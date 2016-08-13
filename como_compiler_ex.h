#ifndef COMO_COMPILER_H
#define COMO_COMPILER_H

#include <stddef.h>
#include <object.h>

#define COMO_DEFAULT_FRAME_STACKSIZE   2048U

typedef struct ComoOpCode {
    unsigned char op_code;
    Object       *operand;
} ComoOpCode;

typedef struct ComoFrame {
    size_t     cf_sp;                      /* stack pointer into cf_stack */
    size_t     cf_stack_size;              /* stack size, num of used entries */
    Object     *cf_stack[(size_t)COMO_DEFAULT_FRAME_STACKSIZE];  /* stack */
    Object     *cf_symtab;               /* Map, symbol table */
    Object     *code;
    struct ComoFrame *next;
    Object *namedparameters;
    Object *filename;
} ComoFrame;

typedef void(*como_vm_executor_t)(ComoFrame *, ComoFrame *);

extern int como_ast_create(const char *filename);

extern como_vm_executor_t *ex;

#endif
