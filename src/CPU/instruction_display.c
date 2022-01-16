#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "CPU/instruction.h"
#include "utils/macro.h"

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
    char *operands = NULL;

    // print instruction's name
    printf("[" HEX "] %-4.4s ", in.pc, instruction_names[in.instruction]);

    switch (in.type) {
    case R16:
    case R8:
        asprintf(&operands, "%s", register_names[in.reg1]);
        break;

    case FLAG:
        asprintf(&operands,"%s, ", in.condition ? "TRUE" : "FALSE");
        break;

    case FLAG_S8:
        asprintf(&operands,"%s, ", in.condition ? "TRUE" : "FALSE");
    case S8:
        asprintf(&operands,"%i", (i8)in.data);
        break;

    case FLAG_A16:
        asprintf(&operands,"%s, ", in.condition ? "TRUE" : "FALSE");
    case A16:
        asprintf(&operands,HEX, in.address);
        break;

    case HL_IMM:
        asprintf(&operands,"(HL)");
        break;

    case RST:
        asprintf(&operands,HEX, in.data);
        break;

    case R8_R8:
        asprintf(&operands,"%s, %s", register_names[in.reg1], register_names[in.reg2]);
        break;

    case R8_D8:
        asprintf(&operands,"%s, " HEX8, register_names[in.reg1], in.data);
        break;

    case R8_HL_REL:
        asprintf(&operands,"%s, (HL)", register_names[in.reg1]);
        break;

    case A_R16_REL:
        asprintf(&operands,"A, (%s)", register_names[in.reg1]);
        break;

    case A_D16_REL:
        asprintf(&operands,"A, (" HEX ")", in.data);
        break;

    case HL_REL_R8:
        asprintf(&operands,"(HL), %s", register_names[in.reg1]);
        break;

    case HL_REL_D8:
        asprintf(&operands,"(HL), " HEX8, in.data);
        break;

    case R16_REL_A:
        asprintf(&operands,"(%s), A", register_names[in.reg1]);
        break;

    case D16_REL_A:
        asprintf(&operands,"(" HEX "), A", in.data);
        break;

    case R16_D16:
        asprintf(&operands,"%s, " HEX, register_names[in.reg1], in.data);
        break;

    case D16_REL_SP:
        asprintf(&operands,"(" HEX "), SP", in.data);
        break;

    case SP_HL:
        asprintf(&operands,"SP, HL");
        break;

    case HLI_A:
        asprintf(&operands,"(HL+), A");
        break;

    case HLD_A:
        asprintf(&operands,"(HL-), A");
        break;

    case A_HLI:
        asprintf(&operands,"A, (HL+)");
        break;

    case A_HLD:
        asprintf(&operands,"A, (HL-)");
        break;

    case NO_OPERAND:
    default:
        asprintf(&operands, "   ");
        break;
    }

    // print operands
    printf("%-12.32s", operands);
    free(operands);

    // print content of the registers
    printf("AF=" HEX " BC=" HEX " DE=" HEX " HL=" HEX "\n",
           read_register_16(REG_AF), read_register_16(REG_BC),
           read_register_16(REG_DE), read_register_16(REG_HL));
}
