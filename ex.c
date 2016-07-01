#include <object.h>

#define is_numeric(o) \
	(O_TYPE((o)) == IS_LONG || O_TYPE((o)) == IS_DOUBLE)

#define one_is_double(left, right) \
	(O_TYPE(left) == IS_DOUBLE || O_TYPE(right) == IS_DOUBLE)

Object *object_concat_helper(Object *left, Object *right) {
	Object *retval;
	
	if(is_numeric(left) && is_numeric(right)) {	
		if(is_one_double(left, right)) {
			double result;
		}	else {
			long result;
		}	
	} else {
		// CONVERT EVERYTHING TO STRING AND stringCat
	}
	
	return retval;	
}
