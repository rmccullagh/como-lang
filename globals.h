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

#ifndef COMO_GLOBALS_H
#define COMO_GLOBALS_H

#include <stddef.h>

#define COMO_FAILURE 1
#define COMO_SUCCESS 0

//#define COMO_DEBUG 1

typedef struct como_global {
	char* filename;
	size_t filename_length;
} como_global;

extern const char* get_file_name();

extern void dump_fn_call_stack();

#endif
