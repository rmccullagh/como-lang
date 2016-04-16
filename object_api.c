#include <object.h>
#include <string.h>
#include "object_api.h"

int object_value_is_less_than(Object *left, Object *right)
{
	if(O_TYPE(left) != O_TYPE(right)) {
		return 0;
	} else {
		switch(O_TYPE(left)) {
			case IS_NULL:
				return 0;
			break;
			case IS_BOOL:
				return O_BVAL(left) < O_BVAL(right);
			break;
			case IS_LONG:
				return O_LVAL(left) < O_LVAL(right);
			break;
			case IS_DOUBLE:
				return O_DVAL(left) < O_DVAL(right);
			break;
			case IS_STRING:
				return strcmp(O_SVAL(left)->value, O_SVAL(right)->value) < 0;	
			break;
			case IS_FUNCTION:
				return O_FVAL(left) < O_FVAL(right);
			break;
			default:
				return 0;
			break;	
		}
	}
}

int object_value_is_greater_than(Object *left, Object *right)
{
	if(O_TYPE(left) != O_TYPE(right)) {
		return 0;
	} else {
		switch(O_TYPE(left)) {
			case IS_NULL:
				return 0;
			break;
			case IS_BOOL:
				return O_BVAL(left) > O_BVAL(right);
			break;
			case IS_LONG:
				return O_LVAL(left) > O_LVAL(right);
			break;
			case IS_DOUBLE:
				return O_DVAL(left) > O_DVAL(right);
			break;
			case IS_STRING:
				return strcmp(O_SVAL(left)->value, O_SVAL(right)->value) > 0;	
			break;
			case IS_FUNCTION:
				return O_FVAL(left) > O_FVAL(right);
			break;
			default:
				return 0;
			break;	
		}
	}
}

