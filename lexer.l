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

D       [0-9]
NUM 		[0-9]+
WHITE_SPACE	[ \r\n\t]*
L		[a-zA-Z_]
A		[a-zA-Z_0-9]

%%

{WHITE_SPACE}	  { /* Skipping Blanks Today */ }
"class"         { return T_CLASS;             }
"func"          { return T_FUNC;              }
"return"        { return T_RETURN;            }
"new"           { return T_NEW;               }
{L}{A}*         {
	size_t len = strlen(yytext);
	yylval->id = malloc(len + 1);
	memcpy(yylval->id, yytext, len + 1);
	return T_ID;
}
{NUM}		        { yylval->number = strtol(yytext, NULL, 10); return T_NUM; }
{D}*"."{D}+     { yylval->dval = strtod(yytext, NULL); return T_DOUBLE;    }

L?\"(\\.|[^\\"])*\"	{ 
	size_t len = strlen(yytext);
	yylval->stringliteral = malloc((len -2) + 1);
	memcpy(yylval->id, yytext + 1, len - 2);
	yylval->id[len - 2] = '\0';
	return T_STR_LIT;
}
.		{ return yytext[0];				     }

%%

