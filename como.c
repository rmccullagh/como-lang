/*
 * Copyright (c) 2016 Ryan McCullagh <me@ryanmccullagh.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <easyio.h>
#include "ast.h"
#include "globals.h"
#include "parser.h"
#include "lexer.h"

static como_global globals;

static void init_globals(void) {
	globals.filename = NULL;
	globals.filename_length = 0;
};

static void set_file_name(const char* name) {
	size_t len = strlen(name);
	globals.filename = malloc(len + 1);
	memcpy(globals.filename, name, len);
	globals.filename[len] = '\0';
	globals.filename_length = len;	
}

const char* get_file_name(void) {
	return globals.filename;
}

int yyparse(ast_node** ast, yyscan_t scanner);

ast_node* como_ast_create(const char* text)
{
	ast_node* statements;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	
	if(yylex_init(&scanner)) {
		return NULL;
	}

	state = yy_scan_string(text, scanner);

	if(yyparse(&statements, scanner)) {
		return NULL;
	}

	yy_delete_buffer(state, scanner);

	yylex_destroy(scanner);

	return statements;
}

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("Usage: ./%s FILE\n", argv[0]);
		return 0;
	}

	char* text = file_get_contents(argv[1]);

	if(!text) {
		printf("file '%s' not found\n", argv[1]);
		return 1;
	}

	init_globals();

	set_file_name(argv[1]);

	ast_node* program = como_ast_create(text);

	free(text);

	
	ast_compile(argv[0], program);

	ast_node_free(program);

	return 0;
}





