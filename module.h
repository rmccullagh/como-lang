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

#ifndef MODULE_H
#define MODULE_H

#include <object.h>

#define COMO_FUNC_NAME(n) como_##n
#define COMO_FUNC_ARGS Object* args, Object** retval
#define COMO_FUNC(name) void COMO_FUNC_NAME(name)(COMO_FUNC_ARGS)

typedef void(*como_native_func)(COMO_FUNC_ARGS);

typedef struct como_function como_function;

typedef struct {
	const char* name;
	const como_function* functions;
} como_module;

struct como_function {
	const char* name;
	void(*handler)(COMO_FUNC_ARGS);
};

typedef struct como_module_list {
	void* handle;
	como_module* module_info;
} como_module_list;

#endif
