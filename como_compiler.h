#ifndef COMO_COMPILER_H
#define COMO_COMPILER_H

#include <stddef.h>
#include <object.h>

#define COMO_DEFAULT_FRAME_STACKSIZE   1024U
#define COMO_DEFAULT_OP_ARRAY_CAPACITY 512U

#define COMO_NODE_OP_IS_LONG       0x01
#define COMO_NODE_OP_IS_DOUBLE     0x02
#define COMO_NODE_OP_IS_JMP_OFFSET 0x03
#define COMO_NODE_OP_IS_STRING     0x04

typedef struct compiler_context compiler_context;

typedef union como_node_op_operand {
	long   lconstant;
	double dconstant; 
	size_t jmp_offset;
	struct {
		size_t len;
		char *value;
	} sconstant;
} como_node_op_operand;

typedef struct como_node_op {
	unsigned char        inst;
	unsigned char 		 op1type;
	como_node_op_operand op1;
} como_node_op;

typedef struct op_array {
	size_t         size;
	size_t         capacity;
	como_node_op   **table;
} op_array;

typedef struct como_frame {
	size_t  cf_sp;
	Object  *cf_stack[COMO_DEFAULT_FRAME_STACKSIZE];
	Object  *cf_symtab;
	Object  *cf_retval;
	op_array cf_code;
} como_frame;

extern
como_frame *como_frame_new(void);

extern como_node_op *
como_make_node(unsigned char inst, unsigned char operand_type);

extern
void emit(unsigned char op, Object *arg);

#endif /* !COMO_COMPILER_H */