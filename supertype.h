#include "comotype2.h"

como_type_spec ComoSuperType = {
 { 
	 { NULL, NULL, 1, &ComoType_Type },
	 0
 },
 "super",
  sizeof(superobject),
	0,
  0,
	0,
	0,
	0,
  super_new
};

