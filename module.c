#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <object.h>

#include "module.h"

como_module* load_module(const char* name)
{
	void* handle;
	como_module* retval;
	como_module* (*module_init)(void);
	char* error;

	handle = dlopen(name, RTLD_LAZY);

	if(!handle) {
		printf("%s\n", dlerror());
		return NULL;
	}		

	module_init = dlsym(handle, "module_init");

	if((error = dlerror()) != NULL) {
		printf("%s\n", error);
		return NULL;
	}
	
	retval = module_init();

	//dlclose(handle);

	return retval;
}

int main(void)
{
	como_module* mod_info = load_module("/home/this/como-src/ext.so");
	size_t i;

	if(!mod_info) {
		return 1;
	}

	printf("loaded module ext.so\n");

	for(i = 0; mod_info->functions[i].name != NULL; i++) {

		printf("%s\n", mod_info->functions[i].name);
	}
	
	return 0;
}


