#include <stdlib.h>
#include "stack.h"

int como_stack_push(como_stack** stack, void* value)
{
	if(*stack == NULL) {
		como_stack* _list = malloc(sizeof(como_stack));
		_list->value = value;
		_list->next = NULL;
		*stack = _list;				
	} else {
		como_stack* prev = (*stack);
		como_stack* next = malloc(sizeof(como_stack));
		next->value = value;
		next->next = prev;
		(*stack) = next;							
	}
	
	return 1;
}

