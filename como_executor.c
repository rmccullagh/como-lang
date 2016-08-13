#include <stdlib.h>
#include <stdio.h>
#include <object.h>
#include "como_opcode.h"
#include "comodebug.h"
#include "como_compiler_ex.h"

void como_vm(ComoFrame *frame, ComoFrame *callingframe) {
    size_t i;
    for(i = 0; i < O_AVAL(frame->code)->size; i++) {
        ComoOpCode *opcode = ((ComoOpCode *)(O_PTVAL(O_AVAL(frame->code)->table[i])));

        switch(opcode->op_code) {
            default: {
                fprintf(stderr, "Invalid OpCode got %d\n", opcode->op_code);
                exit(1);
            }
            case HALT: {
                break;
            }
            case LOAD_CONST: {
                push(frame, opcode->operand);
                break;
            }
            case STORE_NAME: {
                Object *value = pop(frame);
                mapInsert(frame->cf_symtab, 
                    O_SVAL(opcode->operand)->value, value);
                break;
            }
            case LOAD_NAME: {
                Object *value = NULL;
                value = mapSearchEx(frame->cf_symtab, 
                    O_SVAL(opcode->operand)->value);
                if(value) {
                    goto load_name_leave;
                } else {
                    value = mapSearchEx(global_frame->cf_symtab, 
                        O_SVAL(opcode->operand)->value);
                }

                if(value == NULL) {
                    como_error_noreturn("undefined variable '%s'", 
                        O_SVAL(opcode->operand)->value);
                }
load_name_leave:
                push(frame, value);
                break;
            }
            case CALL_FUNCTION: {
                Object *fn = pop(frame);
                Object *argcount = pop(frame);
                long i = O_LVAL(argcount);
                ComoFrame *fnframe;
                if(O_TYPE(fn) != IS_POINTER) {
                    como_error_noreturn("name '%s' is not callable",
                        O_SVAL(opcode->operand)->value);
                }
                fnframe = (ComoFrame *)O_PTVAL(fn);
                if(O_LVAL(argcount) != (long)(O_AVAL(fnframe->namedparameters)->size)) {
                    como_error_noreturn("callable '%s' expects %ld arguments, but %ld were given",
                        O_SVAL(opcode->operand)->value, 
                        (long)(O_AVAL(fnframe->namedparameters)->size), 
                        O_LVAL(argcount));
                }
                como_debug("calling '%s'", O_SVAL(opcode->operand)->value);
                mapInsertEx(fnframe->cf_symtab, "__FUNCTION__", 
                    newString(O_SVAL(opcode->operand)->value));

                while(i--) {
                    como_debug("getting %ldth argument for function call '%s'",
                        i, O_SVAL(opcode->operand)->value);
                    Object *argname = O_AVAL(fnframe->namedparameters)->table[i];

                    Object *argvalue = pop(frame);
                    mapInsert(fnframe->cf_symtab, O_SVAL(argname)->value,
                        argvalue);
                    char *argvaluestr = objectToString(argvalue);
                    como_debug("%ldth argument: '%s' has value: %s", i, O_SVAL(argname)->value,
                        argvaluestr);
                    free(argvaluestr);
                }
                ComoFrame *prev = frame;

                fnframe->next = prev;

                ex(fnframe, prev);
                push(frame, pop(fnframe));
                como_debug("function stack size: %zu", fnframe->cf_stack_size);
                como_debug("function stack pointer: %zu", fnframe->cf_sp);

                fnframe->next = NULL;

                break;
            }
            case ITIMES: {
                Object *right = pop(frame);
                Object *left = pop(frame);
                if(O_TYPE(right) != IS_LONG && O_TYPE(left) != IS_LONG) {
                    como_error_noreturn("invalid operands for ITIMES");
                }
                long value = O_LVAL(left) * O_LVAL(right);
                push(frame, newLong(value));
                break;
            }
            case IRETURN: {
                if(! (O_LVAL(opcode->operand))) {
                    push(frame, newLong(0L));
                }
                return;
            }
            case IPRINT: {
                Object *value = pop(frame);
                size_t len = 0;
                char *sval = objectToStringLength(value, &len);
                fprintf(stdout, "%s\n", sval);
                fflush(stdout);
                free(sval);
                break;          
            }
        }
    }
}