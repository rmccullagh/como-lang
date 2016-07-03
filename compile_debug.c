#include <stdio.h>
#include <stdlib.h>
#include <object.h>

#include "como_opcode.h"
#include "como_compiler.h"

void debug_code_to_output(FILE *fp, compiler_context *cg) {
		fprintf(fp, "*** BEGIN CODE ***\n");
		size_t i;
		for(i = 0; i < cg->code.size; i++) {
			op_code *c = cg->code.table[i];
			switch(c->inst.opcode) {
				case IRETURN: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tIRETURN %s\n", value);
					free(value);
					break;
				}
				case CALL_FUNCTION: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tCALL_FUNCTION %s\n", value);
					free(value);
					break;
				}
				case DEFINE_FUNCTION: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tDEFINE_FUNCTION %s\n", value);
					free(value);
					break;
				}
				case LOAD_CONST: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tLOAD_CONST %s\n", value);
					free(value);
					break;
				}
				case STORE_NAME: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tSTORE_NAME %s\n", value);
					free(value);
					break;
				}
				case LOAD_NAME: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tLOAD_NAME %s\n", value);
					free(value);
					break;
				}
				case IS_LESS_THAN: {
					fprintf(fp, "\tIS_LESS_THAN\n");
					break;
				}
				case JZ: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tJZ %s\n", value);
					free(value);
					break;
				}
				case IPRINT: {
					fprintf(fp,"\tPRINT\n");
					break;
				}
				case IADD: {
					fprintf(fp, "\tADD\n");
					break;
				}
				case IMINUS: {
					fprintf(fp, "\tSUB\n");
					break;
				}
				case IDIV: {
					fprintf(fp, "\tDIV\n");
					break;
				}
				case ITIMES: {
					fprintf(fp, "\tTIMES\n");
					break;
				}
				case JMP: {
					char *value = objectToString(c->op1);
					fprintf(fp, "\tJMP %s\n", value);
					free(value);
					break;
				}
				case NOP: {
					fprintf(fp,"\tNOP\n");
					break;
				}
				case LABEL: {
					char *value = objectToString(c->op1);
					fprintf(fp, "LABEL %s\n", value);
					free(value);
					break;
				}
				case HALT: {
					fprintf(fp, "\tHALT\n");	
					break;
				}
				case IS_EQUAL: {
					fprintf(fp, "\tIS_EQUAL\n");
					break;
				}
				case IS_GREATER_THAN: {
					fprintf(fp, "\tIS_GREATER_THAN\n");
					break;
				}
				case IS_NOT_EQUAL: {
					fprintf(fp, "\tIS_NOT_EQUAL\n");
					break;
				}
				case IS_GREATER_THAN_OR_EQUAL: {
					fprintf(fp, "\tIS_GREATER_THAN_OR_EQUAL\n");
					break;
				}
				case IS_LESS_THAN_OR_EQUAL: {
					fprintf(fp, "\tIS_LESS_THAN_OR_EQUAL\n");
					break;
				}
				case POSTFIX_INC: {
					fprintf(fp, "\tPOSTFIX_INC\n");
					break;
				}
				case UNARY_MINUS: {
					fprintf(fp, "\tUNARY_MINUS\n");
					break;					
				}
			}
		}
		fprintf(fp, "*** END CODE ***\n");
}