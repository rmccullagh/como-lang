%{
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

#include "globals.h"
#include "ast.h"
#include "parser.h"
#include "lexer.h"

#define YYERROR_VERBOSE

int yyerror(YYLTYPE * lvalp, ast_node** ast, yyscan_t scanner, const char* msg)
{
	printf("parse error: %s in file \"%s\" on line %d:%d\n", msg, get_file_name(), lvalp->first_line, lvalp->first_column);
	
	exit(1);
}

%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%output  "parser.c"
%defines "parser.h"

%pure-parser
%locations 
%error-verbose
%expect 0
%lex-param   { yyscan_t scanner }
%parse-param { ast_node** ast }
%parse-param { yyscan_t scanner }

%union {
	double number;
	char* id;
	char* stringliteral;
	ast_node* ast;
} 

%left T_CMP
%precedence '='
%left '-'
%left '+'
%left '*'
%left '/'

%token END 0 "EOF"
%token '-'
%token '+'
%token '*'
%token '/'

%token T_IF
%token T_ELSE
%token T_WHILE
%token T_FUNC
%token T_RETURN
%token T_CMP
%token T_PRINT
%token T_NOELSE


%token <number> T_NUM
%token <id> T_ID
%token <stringliteral> T_STR_LIT

%type <ast> top_statement statement expression_statement compound_statement selection_statement
%type <ast> inner_statement if_statement_without_else
%type <ast> expr
%type <ast> top_statement_list inner_statement_list
%type <ast> optional_parameter_list parameter_list
%type <ast> parameter
%type <ast> function_decl_statement
%type <ast> optional_argument_list argument_list argument
%type <ast> return_statement optional_expression

%%

start:
 top_statement_list { *ast = $1; }
;

top_statement_list:
 top_statement_list top_statement { ast_node_statement_list_push($1, $2); $$ = $1; }
 | %empty { $$ = ast_node_create_statement_list(0); }
;

top_statement:
 statement { $$ = $1; }
;

inner_statement_list:
 inner_statement_list inner_statement { ast_node_statement_list_push($1, $2); $$ = $1; }
 | %empty { $$ = ast_node_create_statement_list(0); } 
;

inner_statement:
 statement { $$ = $1; }
;

statement:
 function_decl_statement { $$ = $1; }
 |
 compound_statement   { $$ = $1; }
 |
 expression_statement { $$ = $1; }
 |
 selection_statement  { $$ = $1; }
 |
 return_statement { $$ = $1; }
;

return_statement:
 T_RETURN optional_expression ';' { $$ = ast_node_create_return($2); }
;

optional_expression:
 expr { $$ = $1; }
 | %empty { $$ = NULL; }
;

compound_statement:
 '{' inner_statement_list '}' { $$ = $2; }
;

expression_statement:
 expr ';' { $$ = $1; }
;

if_statement_without_else:
 T_IF '(' expr ')' compound_statement { $$ = ast_node_create_if($3, $5, NULL); }
;

selection_statement:
 if_statement_without_else %prec T_NOELSE { $$ = $1; }
 |
 if_statement_without_else T_ELSE compound_statement { $1->u1.if_node.b2 = $3; $$ = $1; }
 |
 T_WHILE '(' expr ')' compound_statement {
 	$$ = ast_node_create_while($3, $5);
 }
;

function_decl_statement:
 T_FUNC T_ID '('optional_parameter_list')' compound_statement {
	$$ = ast_node_create_function($2, $4, $6);
	free($2);
 }
;

optional_parameter_list:
 parameter_list { $$ = $1; }
 |
 %empty { $$ = ast_node_create_statement_list(0); }
;

parameter_list:
 parameter                    { $$ = ast_node_create_statement_list(1, $1); }
 | 
 parameter_list ',' parameter { ast_node_statement_list_push($1, $3); $$ = $1; }
;

parameter:
 T_ID { $$ = ast_node_create_id($1); free($1); }
;

optional_argument_list:
 argument_list { $$ = $1; }
 |
 %empty { $$ = ast_node_create_statement_list(0); }
;

argument_list:
 argument { $$ = ast_node_create_statement_list(1, $1); }
 |
 argument_list ',' argument { ast_node_statement_list_push($1, $3); $$ = $1; }
;

argument:
 expr { $$ = $1; }
;

 
expr:
 expr '+' expr   { $$ = ast_node_create_binary_op(AST_BINARY_OP_ADD, $1, $3);   }
 |
 expr '-' expr   { $$ = ast_node_create_binary_op(AST_BINARY_OP_MINUS, $1, $3); }
 |
 expr '*' expr   { $$ = ast_node_create_binary_op(AST_BINARY_OP_TIMES, $1, $3); }
 |
 expr '/' expr   { $$ = ast_node_create_binary_op(AST_BINARY_OP_DIV, $1, $3);   }
 |
 '(' expr ')'    { $$ = $2; }
 |
 T_NUM           { $$ = ast_node_create_number($1); }
 |
 T_ID            { $$ = ast_node_create_id($1);  free($1); }
 |
 T_STR_LIT       { $$ = ast_node_create_string_literal($1); free($1); }
 |
 T_ID '=' expr   { $$ = ast_node_create_binary_op(AST_BINARY_OP_ASSIGN, ast_node_create_id($1), $3); free($1); }
 |
 expr T_CMP expr { $$ = ast_node_create_binary_op(AST_BINARY_OP_CMP, $1, $3);   }
 |
 T_ID '(' optional_argument_list ')' {
	$$ = ast_node_create_call(ast_node_create_id($1), $3, @1.first_line, @1.first_column);
        free($1);
 }
 |
 T_PRINT '(' expr ')' { $$ = ast_node_create_print($3); }
;

%%


