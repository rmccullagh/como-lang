#ifndef STACK_H
#define STACK_H

typedef struct como_stack {
	void* value;
	struct como_stack* next;
} como_stack;

extern int como_stack_push(como_stack** stack, void* value);

#endif
