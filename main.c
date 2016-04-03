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
#include "ast.h"

int main(void)
{

	ast_node* n1 = ast_node_create_number(25);
	ast_node* n2 = ast_node_create_number(10);

	ast_node* op = ast_node_create_binary_op(AST_BINARY_OP_TIMES, n1, n2);

	ast_node_dump_tree(op);

	ast_node_free(op);
	
	return 0;
}
