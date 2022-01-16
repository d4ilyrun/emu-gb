#include <stdio.h>

#include "CPU/instruction.h"

static char *instruction_names[] = {
    "???", "NOP",  "LD",   "PUSH", "POP", "JP",  "JR",  "CALL", "RET", "RETI",
    "RST", "HALT", "STOP", "DI",   "EI",  "CCF", "SCF", "DAA",  "CPL", "ADD",
    "ADC", "SUB",  "SBC",  "INC",  "DEC", "MUL", "AND", "OR",   "XOR"};

char *condition_names[] = {"NZ", "Z", "NC", "C", "???"};

char *register_names[] = {"A",  "F",  "B",  "C",  "D",  "E",  "H",  "L",
                          "PC", "SP", "AF", "BC", "DE", "HL", "???"};

#define HEX8 "0x%02X"
#define HEX16 "0x%04X"
#define HEX HEX16

// TODO: add nice colors :^)
void display_instruction(struct instruction in)
{
    // print instruction's name
    printf("[" HEX "] %s \t", in.pc, instruction_names[in.instruction]);

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
        printf(HEX, in.address);
        break;

    case HL_IMM:
        printf("(HL)");
        break;

    case RST:
        printf(HEX, in.data);
        break;

    case R8_R8:
        printf("%s, %s", register_names[in.reg1], register_names[in.reg2]);
        break;

    case R8_D8:
        printf("%s, " HEX8, register_names[in.reg1], in.data);
        break;

    case R8_HL_REL:
        printf("%s, (HL)", register_names[in.reg1]);
        break;

    case A_R16_REL:
        printf("A, (%s)", register_names[in.reg1]);
        break;

    case A_D16_REL:
        printf("A, (" HEX ")", in.data);
        break;

    case HL_REL_R8:
        printf("(HL), %s", register_names[in.reg1]);
        break;

    case HL_REL_D8:
        printf("(HL), " HEX8, in.data);
        break;

    case R16_REL_A:
        printf("(%s), A", register_names[in.reg1]);
        break;

    case D16_REL_A:
        printf("(" HEX "), A", in.data);
        break;

    case NO_OPERAND:
    default:
        break;
    }

    // print content of the registers
    printf("\tBC=" HEX " DE=" HEX " HL=" HEX "\n", read_register_16(REG_BC),
           read_register_16(REG_DE), read_register_16(REG_HL));
}
