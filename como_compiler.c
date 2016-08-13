#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include "como_compiler.h"
#include "comodebug.h"
#include "como_opcode.h"

como_frame *como_frame_create(void)
{
	size_t i;
	como_frame *frame = malloc(sizeof(como_frame));

	if(frame == NULL) {
		COMO_OOM();
	}

	frame->cf_sp = 0;
	frame->cf_stack_size = 0;
	frame->cf_fname = NULL;

	for(i = 0; i < (size_t)COMO_DEFAULT_FRAME_STACKSIZE; i++) {
		frame->cf_stack[i] = NULL;
	}

	frame->cf_scope = NULL;
	frame->cf_symtab = newMap(4);
	frame->cf_current_symtab = NULL;
	frame->cf_retval = NULL;

	return frame;
}

void como_frame_free(como_frame *frame)
{
	size_t i;

	if(frame->cf_fname != NULL) {
		objectDestroy(frame->cf_fname);
	}

	for(i = 0; i < (size_t)COMO_DEFAULT_FRAME_STACKSIZE; i++) {
		if(frame->cf_stack[i] != NULL) {
			objectDestroy(frame->cf_stack[i]);
		}
	}

	if(frame->cf_symtab != NULL) {
		objectDestroy(frame->cf_symtab);
	}

	if(frame->cf_retval != NULL) {
		objectDestroy(frame->cf_retval);
	}

	free(frame);
}

como_op_array *como_op_array_create(void)
{
	size_t i;
	como_op_array *o = malloc(sizeof(como_op_array));

	if(o == NULL) {
		COMO_OOM();
	}

	o->call_stack = NULL;
	o->frame = como_frame_create();
	o->size = 0;
	o->capacity = (size_t)COMO_DEFAULT_OP_ARRAY_CAPACITY;
	o->pc = 0;
	o->table = calloc((size_t)COMO_DEFAULT_OP_ARRAY_CAPACITY, sizeof(como_op_code));

	if(o->table == NULL) {
		como_frame_free(o->frame);
		free(o);
		COMO_OOM();
	}

	for(i = 0; i < (size_t)COMO_DEFAULT_OP_ARRAY_CAPACITY; i++) {
		o->table[i] = NULL;
	}

	return o;
}

void como_emit(como_op_array* op_array, unsigned char op_code, Object *operand)
{
	como_op_code *i = malloc(sizeof(como_op_code));

	if(i == NULL) {
		COMO_OOM();
	}

	i->op_code = op_code;
	i->operand = operand;

	if(op_array->size >= op_array->capacity) {
		como_debug("resizing op_array->table");
		size_t new_capacity = op_array->capacity * 2;
		void *new_array = realloc(op_array->table, sizeof(como_op_code) * new_capacity);

		if(new_array == NULL) {
			COMO_OOM();
		}

		op_array->table = new_array;
		op_array->capacity = new_capacity;
	}

	op_array->table[op_array->size++] = i;
}


