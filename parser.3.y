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
	double dval;
	char* id;
	char* stringliteral;
	ast_node* ast;
}

%token '.'
%token '='
%token '+'
%token T_NEW
%token <number> T_NUM
%token <dval> T_DOUBLE
%token <id> T_ID
%token <stringliteral> T_STR_LIT

%type <ast> expression statement expression_statement statement_list
%type <ast> value primary call accessor optional_arg_list
%type <ast> argument_list argument

%left '=' '+'

%%


start
    : statement_list              { *ast = $1;                             }
    ;

optional_arg_list
    : %empty        { $$ = ast_node_create_statement_list(0); }
    | argument_list { $$ = $1;                                }
    ;

argument_list
    : argument      { $$ = ast_node_create_statement_list(1, $1); }
    | argument_list ',' argument {
        ast_node_statement_list_push($1, $3);
        $$ = $1;
    }
    ;

argument
    : expression    { $$ = $1; } 
    ;

primary
    : value { $$ = $1;    }
    | accessor { $$ = $1; }
    | call { $$ = $1;     }
    ;

call
    : primary '('optional_arg_list ')' {
		     $$ = ast_node_create_call($1, $3, 
                @1.first_line);
    }
    ;

accessor
    : primary '.' T_ID {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_DOT, $1, 
                  ast_node_create_id($3, @3.first_line), 
                  @1.first_line);
             free($3);
    }
    ;

value
    : T_NUM                       { $$ = ast_node_create_number($1, @1.first_line);       }
    | T_DOUBLE                    { $$ = ast_node_create_double($1, @1.first_line);       }
    | T_ID                        { $$ = ast_node_create_id($1, @1.first_line); free($1);                }
    | T_STR_LIT                   { 
        $$ = ast_node_create_string_literal($1, @1.first_line); 
        free($1); 
    }
    ;

expression
    : value                       { $$ = $1;                               }
    | T_ID       '=' expression   { 
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ASSIGN,
               ast_node_create_id($1, @1.first_line), $3, @1.first_line);
             free($1);
    }
    | accessor { $$ = $1; }
    | call     { $$ = $1; }
    | expression '+' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ADD, $1, $3, @1.first_line);
    }
    | '(' expression ')' { $$ = $2; }
    ;

statement
    : expression_statement        { $$ = $1;                                }
    ;

statement_list
    : %empty                      { $$ = ast_node_create_statement_list(0); } 
    | statement_list statement    {
        ast_node_statement_list_push($1, $2);
        $$ = $1;
    }
    ;

expression_statement
    : expression ';'              { $$ = $1;                                }
    ;

%%








