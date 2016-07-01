#include <object.h>

#define is_numeric(o) \
	(O_TYPE((o)) == IS_LONG || O_TYPE((o)) == IS_DOUBLE)

#define one_is_double(left, right) \
	(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE)

#define EXTRACT_KNOWN_NUMERIC_AS_DOUBLE(x) \
	(O_TYPE((x)) == IS_DOUBLE ? O_DVAL((x)) : (double)O_LVAL((x)))

Object *object_concat_helper(Object *left, Object *right) {
	Object *retval;
	
	if(is_numeric(left) && is_numeric(right)) {	
		if(is_one_double(left, right)) {
			double op1, op2;
			op1 = EXTRACT_KNOWN_NUMERIC_AS_DOUBLE(left);
			op2 = EXTRACT_KNOWN_NUMERIC_AS_DOUBLE(right);
			retval = newDouble(op1 + op2);
		}	else {
			retval = newLong(O_LVAL(left) + O_LVAL(right));	
		}	
	} else {
		// CONVERT EVERYTHING TO STRING AND stringCat
	}
	
	return retval;	
}
