#include <stdio.h>

#include "CPU/instruction.h"

static char *instruction_names[] = {
    "???", "NOP",  "LD",   "PUSH", "POP", "JP",  "JR",  "CALL", "RET", "RETI",
    "RST", "HALT", "STOP", "DI",   "EI",  "CCF", "SCF", "DAA",  "CPL", "ADD",
    "ADC", "SUB",  "SBC",  "INC",  "DEC", "MUL", "AND", "OR",   "XOR"};

char *condition_names[] = {"NZ", "Z", "NC", "C", "???"};

char *register_names[] = {"A",  "F",  "B",  "C",  "D",  "E",  "H",  "L",
                          "PC", "SP", "AF", "BC", "DE", "HL", "???"};

// TODO: add nice colors :^)
void display_instruction(struct instruction in)
{
    // print instruction's name
    printf("[0x%04X] %s \t", in.pc, instruction_names[in.instruction]);

    // TODO: print operands
    switch (in.type) {
    case R16:
    case R8:
        printf("%s", register_names[in.reg1]);
        break;

    case FLAG:
        printf("%s, ", in.condition ? "TRUE" : "FALSE");
        break;

    case FLAG_S8:
        printf("%s, ", in.condition ? "TRUE" : "FALSE");
    case S8:
        printf("%i", (i8)in.data);
        break;

    case FLAG_A16:
        printf("%s, ", in.condition ? "TRUE" : "FALSE");
    case A16:
        printf("0x%04X", in.address);
        break;

    case HL_IMM:
        printf("(HL)");
        break;

    case NO_OPERAND:
    default:
        break;
    }

    // print content of the registers
    printf("\tBC=0x%04X DE=0x%04X HL=0x%04X\n", read_register_16(REG_BC),
           read_register_16(REG_DE), read_register_16(REG_HL));
}
