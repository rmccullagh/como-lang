/*
*  Copyright (c) 2016 Ryan McCullagh <me@ryanmccullagh.com>
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
		printf("%s:%s\n", __func__, dlerror());
		return NULL;
	}		

	module_init = dlsym(handle, "module_init");

	if((error = dlerror()) != NULL) {
		printf("%s:%s\n", __func__, error);
		return NULL;
	}
	
	retval = module_init();

	//dlclose(handle);

	return retval;
}

int main(void)
{
	como_module* mod_info = load_module("/home/ryanm/src/como-lang/ext.so");
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


