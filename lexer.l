%{
/*
 * lexer.l
 *
 */

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "ast.h"
#include "parser.h"

static void update_loc(YYLTYPE *loc, char* text)
{
	loc->first_line = loc->last_line;
	loc->first_column = loc->last_column;
	int i;
	for(i = 0; text[i] != '\0'; i++) {
		if(text[i] == '\n') {
			loc->last_line++;
			loc->last_column = 0;
		} else {
			loc->last_column++;
		}
	}
}

#define YY_USER_ACTION update_loc(yylloc, yytext);	

%}

%option outfile="lexer.c" header-file="lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd yylineno
%option bison-bridge
%option bison-locations

NUM 		[0-9]+
WHITE_SPACE	[ \r\n\t]*
L		[a-zA-Z_]
A		[a-zA-Z_0-9]

%%


"if"		{ return T_IF;    }
"else"          { return T_ELSE;  }
"func"          { return T_FUNC;  }
"print"         { return T_PRINT;  }
"return"        { return T_RETURN; }
{WHITE_SPACE}	{ /* Skipping Blanks Today */ }
{L}{A}*         {
	size_t len = strlen(yytext);
	yylval->id = malloc(len + 1);
	memcpy(yylval->id, yytext, len + 1);
	return T_ID;
}
{NUM}		{ yylval->number = atoi(yytext); return T_NUM;       }
.		{ return yytext[0];				     }

%%

