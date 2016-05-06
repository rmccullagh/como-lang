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
%}

%option outfile="lexer.c" header-file="lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd yylineno
%option bison-bridge
%option bison-locations

D   [0-9]
L		[a-zA-Z_]
A		[_0-9]

%%

"if"            { return T_IF;                }
"else"          { return T_ELSE;              }
"!="            { return T_NOT_EQUAL;         }
"=="            { return T_CMP;               }
{L}+{A}*         {
	size_t len = strlen(yytext);
	yylval->id = malloc(len + 1);
	memcpy(yylval->id, yytext, len);
	yylval->id[len] = '\0';
	return T_ID;
}
{D}+		      { yylval->number = strtol(yytext, NULL, 10); return T_NUM; }
{D}*"."{D}+     { yylval->dval = strtod(yytext, NULL); return T_DOUBLE;    }

L?\"(\\.|[^\\"])*\"	{ 
	size_t len = strlen(yytext);
	yylval->stringliteral = malloc((len -2) + 1);
	memcpy(yylval->stringliteral, yytext + 1, len - 2);
	yylval->stringliteral[len - 2] = '\0';
	return T_STR_LIT;
}
[ \t\n]+ 	/* eat white */

.	{ return yytext[0];				     }

%%

