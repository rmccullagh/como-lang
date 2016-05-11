%{

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "globals.h"

#define YYERROR_VERBOSE

int yyerror(YYLTYPE * lvalp, ast_node** ast, 
	yyscan_t scanner, const char* msg)
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

%token '='
%token '-'
%token '+'
%token '*'
%token '/'
%token T_CMP
%token T_IF "if (T_IF)"
%token T_ELSE
%token T_NOELSE
%token T_NOT_EQUAL
%token <number> T_NUM
%token <dval> T_DOUBLE
%token <id> T_ID
%token <stringliteral> T_STR_LIT

%type <ast> expression statement expression_statement statement_list
%type <ast> value assignment_statement
%type <ast> if_statement_without_else selection_statement
%type <ast> optional_argument_list argument_list argument

%nonassoc T_CMP T_NOT_EQUAL
%nonassoc '<' '>'
%left '-' '+'
%left '/' '*'

%%


start
    : statement_list              { *ast = $1;                             }
    ;

value
    : T_NUM                       { $$ = ast_node_create_number($1);       }
    | T_DOUBLE                    { $$ = ast_node_create_double($1);       }
    | T_ID                        { $$ = ast_node_create_id($1); free($1); }
    | T_STR_LIT                   { 
        $$ = ast_node_create_string_literal($1); 
        free($1); 
    }
    ;

expression
    : value                       { $$ = $1;                               }
    | expression T_NOT_EQUAL expression {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_NOT_EQUAL, $1, $3);
    }
    | expression T_CMP expression {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_CMP, $1, $3);
    }
    | expression '<' expression {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_LESS_THAN, $1, $3);
    }
    | expression '>' expression {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_GREATER_THAN, $1, $3);
    }
    | T_ID       '(' optional_argument_list ')' {
        $$ = ast_node_create_call(ast_node_create_id($1), $3, 
                 @1.first_line, @1.first_column);
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

optional_argument_list
    : %empty                      { $$ = ast_node_create_statement_list(0); }
    | argument_list               { $$ = $1;                                }
    ;

argument_list
    : argument                    { 
        $$ = ast_node_create_statement_list(1, $1);
    }
    | argument_list ',' argument  { 
        ast_node_statement_list_push($1, $3); 
        $$ = $1;
    }
    ;

argument
    : expression                  { $$ = $1;                                }
    ;

statement
    : expression_statement        { $$ = $1;                                }
    | selection_statement         { $$ = $1;                                }
		| assignment_statement        { $$ = $1;                                }
    ;

assignment_statement
		:  T_ID  '=' expression ';'  { 
        $$ = ast_node_create_assign(ast_node_create_id($1), $3);
        free($1);
    }
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

if_statement_without_else
    : T_IF '(' expression ')' '{' statement_list '}'  {
        $$ = ast_node_create_if($3, $6, NULL);
    }
    ;

selection_statement
    : if_statement_without_else %prec T_NOELSE { $$ = $1;                   }
    | if_statement_without_else T_ELSE '{' statement_list '}' { 
         $1->u1.if_node.b2 = $4; 
         $$ = $1; 
    }
    ;

%%








