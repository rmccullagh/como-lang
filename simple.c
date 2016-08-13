#include <stdlib.h>
#include <stdio.h>
#include <object.h>
#include "como_opcode.h"
#include "comodebug.h"

#define COMO_DEFAULT_FRAME_STACKSIZE   2048U

typedef struct ComoOpCode {
    unsigned char op_code;
    Object       *operand;
} ComoOpCode;

typedef struct ComoFrame {
    size_t     cf_sp;                      /* stack pointer into cf_stack */
    size_t     cf_stack_size;              /* stack size, num of used entries */
    Object     *cf_stack[(size_t)COMO_DEFAULT_FRAME_STACKSIZE];  /* stack */
    Object     *cf_symtab;               /* Map, symbol table */
    Object     *code;
    struct ComoFrame *next;
    Object *namedparameters;
} ComoFrame;

static ComoFrame *global_frame = NULL;

static inline void push(ComoFrame *frame, Object *value) {
    if(frame->cf_sp >= COMO_DEFAULT_FRAME_STACKSIZE) {
        como_error_noreturn("error stack overflow tried to push onto #%zu", frame->cf_sp);
    } else {
        frame->cf_stack[frame->cf_sp++] = value;
        frame->cf_stack_size++;
    }
}

static inline Object *pop(ComoFrame *frame) {
    Object *retval = NULL;
    if(frame->cf_sp == 0) {
        como_error_noreturn("stack underflow, tried to go before 0");
    } else {
        retval = frame->cf_stack[--frame->cf_sp];
        --frame->cf_stack_size;
    }
    return retval;
}

static ComoFrame *create_frame(Object *code) {
    size_t i;
    ComoFrame *frame = malloc(sizeof(ComoFrame));

    frame->cf_sp = 0;
    frame->cf_stack_size = 0;

    for(i = 0; i < (size_t)COMO_DEFAULT_FRAME_STACKSIZE; i++) {
        frame->cf_stack[i] = NULL;
    }

    frame->cf_symtab = newMap(4);
    frame->code = code;
    frame->next = NULL;
    frame->namedparameters = newArray(2);

    return frame;
}

static ComoOpCode *create_op(unsigned char op, Object *oper) {
    ComoOpCode *ret = malloc(sizeof(ComoOpCode));
    ret->op_code = op;
    ret->operand = oper;
    return ret;
}

static void ex(ComoFrame *frame, ComoFrame *callingframe) {
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

int main(void) {

    Object *main_code = newArray(4);
    arrayPushEx(main_code, newPointer((void *)create_op(LOAD_CONST, newLong(5L)))); 
    arrayPushEx(main_code, newPointer((void *)create_op(LOAD_CONST, newLong(5L)))); 
    arrayPushEx(main_code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(main_code, newPointer((void *)create_op(LOAD_NAME, newString("do_times"))));
    arrayPushEx(main_code, newPointer((void *)create_op(CALL_FUNCTION, newString("do_times"))));

    arrayPushEx(main_code, newPointer((void *)create_op(STORE_NAME, newString("result"))));
    arrayPushEx(main_code, newPointer((void *)create_op(LOAD_NAME, newString("result"))));
    arrayPushEx(main_code, newPointer((void *)create_op(IPRINT, NULL)));
    arrayPushEx(main_code, newPointer((void *)create_op(HALT, NULL)));

    ComoFrame *globalframe = create_frame(main_code);

    Object *do_times_code = newArray(4);
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("__FUNCTION__"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(IPRINT, NULL)));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("left"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("right"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("do_times"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(CALL_FUNCTION, newString("do_times"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(do_times_code, newPointer((void *)create_op(IRETURN, newLong(1L))));

    Object *do_times_parameters = newArray(2);
    arrayPushEx(do_times_parameters, newString("left"));
    arrayPushEx(do_times_parameters, newString("right"));

    ComoFrame *do_times_frame = create_frame(do_times_code);
    do_times_frame->namedparameters = do_times_parameters;

    mapInsertEx(globalframe->cf_symtab, "do_times", newPointer((void *)do_times_frame));
    mapInsertEx(globalframe->cf_symtab, "__FUNCTION__", newString("__main__"));

    global_frame = globalframe;

    (void)ex(globalframe, NULL);

    return 0;
}



