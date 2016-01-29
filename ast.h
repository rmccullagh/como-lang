#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <object.h>
#include <stdarg.h>

/*
 * http://epaperpress.com/lexandyacc/
 */
typedef enum {
	AST_NODE_TYPE_NUMBER, AST_NODE_TYPE_STRING,
	AST_NODE_TYPE_ID, AST_NODE_TYPE_UNDEFINED,
	AST_NODE_TYPE_STATEMENT_LIST, AST_NODE_TYPE_BIN_OP,
	AST_NODE_TYPE_IF, AST_NODE_TYPE_WHILE, AST_NODE_TYPE_FUNC_DECL, 
	AST_NODE_TYPE_CALL, AST_NODE_TYPE_RET, AST_NODE_TYPE_PRINT
} ast_node_type;

typedef enum {
	AST_BINARY_OP_ADD, AST_BINARY_OP_MINUS, AST_BINARY_OP_ASSIGN,
	AST_BINARY_OP_TIMES, AST_BINARY_OP_DIV, AST_BINARY_OP_CMP
} ast_binary_op_type;

typedef struct ast_node ast_node;

/*
 * The pair's first is the variable name as a string
 * The pair's second is a void * pointer casted to an ast_node
 * meaning that variables can hold any ast_node type
 * we must do a check to make sure it doesn't hold a statement
 * or something else weird
 *
 * Or it can hold a bucket index,
 *
 * Variables are defined at declaration time
 *
 * x // error, x is not defined
 * x = 1 // ok
 *
 * A variable can hold a function
 * 
 * There will be on symbol table
 * a Map that holds a void pointer, that will be cast to an ast_node*
 * On variable declaration, insert it into the hash table, the global symbol table
 * Using a hash table provides faster lookup than an O(n) linear search
 * through an array
 *
 * The variable value can be primitive, or be a function
 */
typedef struct {
	char* name;
	size_t length;
} ast_node_id;

typedef struct {
	void* value;
} ast_node_undefined;

typedef enum {
	AST_NODE_RVALUE_TYPE_AST, AST_NODE_RVALUE_TYPE_PRIMITIVE
} ast_node_rvalue_type;

typedef struct {
	ast_node_rvalue_type	type;
	union {
		ast_node*	ast_value;
		Object*		primitive_value;	
	} u1;
} ast_node_rvalue;

typedef struct ast_node_statements ast_node_statements;

struct ast_node_statements {
	size_t	   count;
	size_t	   capacity;
	ast_node** statement_list;
};

typedef struct {
	ast_binary_op_type type;
	ast_node* left;
	ast_node* right;
} ast_node_binary;

typedef struct {
	ast_node* condition;
	ast_node* b1;
	ast_node* b2;
} ast_node_if;

typedef struct {
	ast_node* condition;
	ast_node* body;
} ast_node_while;

/* function a() {} */
typedef struct {
	char* name;
	size_t name_length;
	ast_node* parameter_list;
	ast_node* body;	
} ast_node_function;

typedef struct {
	int lineno;
	int colno;
	ast_node* id;
	ast_node* arguments;
} ast_node_call;

typedef struct {
	ast_node* expr;
} ast_node_return;

typedef struct {
	ast_node* expr;	
} ast_node_print;

struct ast_node {
	ast_node_type	type;
	union {
		double	number_value;
		struct {
			char* value;
			size_t length;
		} string_value;
		ast_node_id		id_node;
		ast_node_undefined	undefined_node;
		ast_node_rvalue		rval_node;
		ast_node_statements	statements_node;
		ast_node_binary		binary_node;
		ast_node_if		if_node;
		ast_node_while		while_node;
		ast_node_function       function_node;
		ast_node_call           call_node;
		ast_node_return		return_node;
		ast_node_print		print_node;
	} u1;
};

extern ast_node* ast_node_create_number(double value);
extern ast_node* ast_node_create_statement_list(size_t count, ...);
extern void ast_node_statement_list_push(ast_node* node, ast_node* value);
extern ast_node* ast_node_create_binary_op(ast_binary_op_type type, ast_node* left, ast_node* right);
extern void ast_node_free(ast_node* node);

extern ast_node* ast_node_create_id(const char* name);

extern ast_node* ast_node_create_if(ast_node* condition, ast_node* b1, ast_node* b2);

extern ast_node* ast_node_create_while(ast_node* condition, ast_node* body);

extern ast_node* ast_node_create_function(const char* name, ast_node* parameters, ast_node* body);

extern ast_node* ast_node_create_call(ast_node* id, ast_node* arguments, int lineno, int col);

extern ast_node* ast_node_create_return(ast_node* expr);

extern ast_node* ast_node_create_print(ast_node* expr);

extern void ast_node_dump_tree(ast_node* node);

extern void ast_compile(const char* file, ast_node* program);

#endif
