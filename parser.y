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

%token T_CLASS
%token T_FUNC
%token T_RETURN
%token T_NEW
%token T_NAMESPACE;
%token '.'
%token '='
%token '+'
%token <number> T_NUM
%token <dval> T_DOUBLE
%token <id> T_ID
%token <stringliteral> T_STR_LIT

%type <ast> expression statement expression_statement statement_list
%type <ast> value primary call accessor optional_arg_list
%type <ast> argument_list argument
%type <ast> target assignment_statement
%type <ast> class_defn_statement class_name optional_parameter_list
%type <ast> parameter_list parameter
%type <ast> function_defn_statement function_statements class_statements class_statement
%type <ast> return_statement optional_expression function_statement

%left '+'

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
                @1.first_line, @1.first_column);
    }
    ;

accessor
    : primary '.' T_ID {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_DOT, $1, 
                  ast_node_create_id($3, @3.first_line, @3.first_column), 
                  @1.first_line, @1.first_column);
             free($3);
    }
    ;

value
    : T_NUM                       { 
          $$ = ast_node_create_number($1, @1.first_line, @1.first_column);       
    }
    | T_DOUBLE                    { 
          $$ = ast_node_create_double($1, @1.first_line, @1.first_column);       
    }
    | T_ID                        { 
          $$ = ast_node_create_id($1, @1.first_line, @1.first_column); 
          free($1);                
    }
    | T_STR_LIT                   { 
        $$ = ast_node_create_string_literal($1, @1.first_line, @1.first_column); 
        free($1); 
    }
    ;

expression
    : primary                     { $$ = $1;                               }
    | expression '+' expression   {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ADD, $1, $3, @1.first_line, @1.first_column);
    }
		| T_NEW class_name '(' optional_arg_list ')' {
			$$ = ast_node_create_new($2, $4, @1.first_line, @1.first_column);
		}
		| '(' expression ')' { $$ = $2; }
    ;

statement
    : expression_statement        { $$ = $1;                                }
    | assignment_statement        { $$ = $1;                                }
    | class_defn_statement        { $$ = $1;                                }
		;

function_statement
    : expression_statement        { $$ = $1;                                }
    | assignment_statement        { $$ = $1;                                }
    | return_statement            { $$ = $1;                                }
		;

class_statement
    : function_defn_statement    { $$ = $1;                                 }
		;

target
    : T_ID { $$ = ast_node_create_id($1, @1.first_line, @1.first_column); 
                  free($1);          
    }
    | accessor { $$ = $1; }
    ;

assignment_statement
    : target '=' expression ';'     {
        $$ = ast_node_create_binary_op(AST_BINARY_OP_ASSIGN, $1, $3, 
                          @1.first_line, @1.first_column);
   	}
		;

class_statements
    : %empty                      { $$ = ast_node_create_statement_list(0); } 
    | class_statements class_statement    {
        ast_node_statement_list_push($1, $2);
        $$ = $1;
    }
    ;

function_statements
     : %empty                      { $$ = ast_node_create_statement_list(0); } 
    | function_statements function_statement    {
        ast_node_statement_list_push($1, $2);
        $$ = $1;
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

return_statement
    : T_RETURN optional_expression ';' { 
				$$ = ast_node_create_return($2, @1.first_line, @1.first_column); 
    }
    ;

class_name
    : T_ID                         {
			$$ = ast_node_create_id($1, @1.first_line, @1.first_column);
      free($1);
    }
    ;

optional_parameter_list
    : %empty { $$ = ast_node_create_statement_list(0); }
    | parameter_list { $$ = $1; }
    ;

parameter_list
    : parameter { $$ = ast_node_create_statement_list(1, $1); }
    | parameter_list ',' parameter { ast_node_statement_list_push($1, $3); $$ = $1; }
    ;

parameter
    : T_ID { $$ = ast_node_create_id($1, @1.first_line, @1.first_column); free($1); }
    ;

optional_expression
    : %empty { $$ = NULL; }
    | expression { $$ = $1; }
    ;

function_defn_statement
		: T_FUNC class_name '(' optional_parameter_list ')' '{' function_statements '}' {
			$$ = ast_node_create_function_defn($2, $4, $7, @1.first_line, @1.first_column);
    }
		;

class_defn_statement
    : T_CLASS class_name '{' class_statements '}' {
			$$ = ast_node_create_class_defn($2, $4, @1.first_line, @1.first_column);
    }
    ;

%%








