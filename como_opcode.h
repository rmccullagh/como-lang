#ifndef COMO_OPCODE_H
#define COMO_OPCODE_H

#define INONE                    0x00
#define LOAD_CONST               0x01
#define STORE_NAME               0x02
#define LOAD_NAME                0x03
#define IS_LESS_THAN             0x04
#define JZ			             0x05
#define IPRINT                   0x06
#define IADD                     0x07
#define JMP                      0x08
#define IRETURN                  0x09
#define NOP                      0x0a
#define LABEL                    0x0b
#define HALT                     0x0c
#define IS_EQUAL                 0x0d
#define IDIV                     0x0e
#define ITIMES                   0x0f
#define IMINUS          		 0x10
#define IS_GREATER_THAN 		 0x11
#define IS_NOT_EQUAL             0x12
#define IS_GREATER_THAN_OR_EQUAL 0x13
#define IS_LESS_THAN_OR_EQUAL    0x14
#define DEFINE_FUNCTION          0x15
#define CALL_FUNCTION            0x16
#define POSTFIX_INC              0x17
#define UNARY_MINUS              0x18
#define IREM					 0x19
#define POSTFIX_DEC              0x20


#endif /* !COMO_OPCODE_H */