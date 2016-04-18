#ifndef COMO_STACK_FRAME_H
#define COMO_STACK_FRAME_H

#include "stack.h"
#include <object.h>

typedef struct como_stack_frame {
	como_stack *caller;
	Object *scope;
	Object *arguments;
	Object **retval;
} como_stack_frame;

#endif
