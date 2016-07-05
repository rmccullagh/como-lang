#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "como_compiler.h"
#include "comodebug.h"

como_node_op *como_make_node(unsigned char inst, unsigned char operand_type)
{
	como_node_op *op = malloc(sizeof(como_node_op));

	if(op == NULL) {
		como_error_noreturn("out of memory");
	}

	op->inst = inst;
	op->op1type = operand_type;

	return op;
}

como_node_op *como_make_long_operand(long value)
{
	como_node_op *op = como_make_node
}

void emit(unsigned char op, Object *arg)
{

}