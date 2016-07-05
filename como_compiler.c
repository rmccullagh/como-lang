#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "como_compiler.h"
#include "comodebug.h"

como_node_op *como_make_node(unsigned char inst, unsigned char operand_type)
{
	como_node_op *op = malloc(sizeof(como_node_op));
	if(op == NULL)
		return op;

	op->inst = inst;
	op->op1type = operand_type;
	return op;
}

como_node_op *como_make_long_operand(unsigned char inst, long value)
{
	como_node_op *op = como_make_node(inst, COMO_NODE_OP_IS_LONG);

	if(op == NULL) {
		como_error_noreturn("out of memory");
	}

	op->op1.lconstant = value;
	return op;
}

como_node_op *como_make_long_operand(unsigned char inst, long value)
{
	como_node_op *op = como_make_node(inst, COMO_NODE_OP_IS_LONG);

	if(op == NULL) {
		como_error_noreturn("out of memory");
	}

	op->op1.lconstant = value;
	return op;
}


void emit(unsigned char op, Object *arg)
{

}