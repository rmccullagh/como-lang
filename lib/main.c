#include <stdlib.h>
#include <stdio.h>

typedef struct como_value como_value;
typedef union como_function_impl_t como_function_impl_t;
//typedef como_value *(*como_type_function)(como_value *, como_value *);

typedef enum como_value_type {
	COMO_TYPE_IS_FUNCTION = 0
} como_value_type;

union como_function_impl_t {
	void *value;
	como_value *(*handler)(como_value *, como_value *);
};

struct como_value {
	como_value_type         type;
	int 		            flags;
	como_function_impl_t    function_value;
};

como_value *como_to_string(como_value *self, como_value *args)
{
	(void)self;
	(void)args;
	return NULL;
}

int main(void)
{
	como_value cv;
	cv.type = COMO_TYPE_IS_FUNCTION;
	cv.flags = 0;
	cv.function_value.handler = como_to_string;


	return 0;
}