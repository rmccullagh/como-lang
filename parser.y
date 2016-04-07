%{

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "globals.h"

#define YYERROR_VERBOSE

int yyerror(YYLTYPE * lvalp, ast_node** ast, yyscan_t scanner, const char* msg)
{
	printf("parse error: %s in file \"%s\" on line %d:%d\n", 
      msg, "<file>", lvalp->first_line, lvalp->first_column);
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
	long number;
	char* id;
	char* stringliteral;
	ast_node* ast;
}

%token '-'
%token '+'
%token '*'
%token '/'
%token T_CMP
%token <number> T_NUM
%token <id> T_ID
%token <stringliteral> T_STR_LIT

%type <ast> expression statement expression_statement statement_list

%precedence '='
%left T_CMP
%left '-' '+'
%left '/' '*'

%%


start
    : statement_list              { *ast = $1;                             }
    ;

expression
    : T_NUM                       { $$ = ast_node_create_number($1);       }
    | T_ID                        { $$ = ast_node_create_id($1); free($1); }
    | T_STR_LIT                   { 
        $$ = ast_node_create_string_literal($1); 
        free($1); 
    }
    | expression T_CMP expression {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_CMP, $1, $3);
    }
    | T_ID       '=' expression   { 
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ASSIGN,
               ast_node_create_id($1), $3);
             free($1);
    }
    | expression '-' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_MINUS, $1, $3);
    }
    | expression '+' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ADD, $1, $3);
    }
    | expression '/' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_DIV, $1, $3);
    }
    | expression '*' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_TIMES, $1, $3);
    }
    | '(' expression ')'          { $$ = $2;                                }
    ;

statement
    : expression_statement       { $$ = $1;                                }
    ;

statement_list
    : %empty                     { $$ = ast_node_create_statement_list(0); } 
    | statement_list statement   {
        ast_node_statement_list_push($1, $2);
        $$ = $1;
    }
    ;

expression_statement
    : expression ';'             { $$ = $1;                                }
    ;

%%

