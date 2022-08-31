#define _GNU_SOURCE // needed for asprintf

#include <stdio.h>
#include <stdlib.h>

#include "CPU/instruction.h"
#include "options.h"
#include "utils/log.h"
#include "utils/macro.h"

#ifdef TEST_ROM
#include "test_rom.h"
#endif

static char *instruction_names[] = {
    "ADC", "ADD",  "AND", "CALL", "CB",   "CCF",  "CP",   "CPL", "DAA",
    "DEC", "DI",   "EI",  "ERR",  "HALT", "INC",  "JP",   "JR",  "LD",
    "LDH", "NOP",  "OR",  "POP",  "PUSH", "RET",  "RETI", "RLA", "RLCA",
    "RRA", "RRCA", "RST", "SBC",  "SCF",  "STOP", "SUB",  "XOR",
};

char *condition_names[] = {"NZ", "Z", "NC", "C", "???"};

char *register_names[] = {"A",  "F",  "B",  "C",  "D",  "E",  "H",  "L",
                          "PC", "SP", "AF", "BC", "DE", "HL", "???"};

void display_instruction(struct instruction in)
{
    if (get_options()->trace == false)
        return;

    char *line = NULL;
    char *operands = NULL;

    switch (in.type) {
    case R16:
    case R8:
        asprintf(&operands, "%s", register_names[in.reg1]);
        break;

    case FLAG:
        asprintf(&operands, "%s, ", in.condition ? "TRUE" : "FALSE");
        break;

    case FLAG_S8:
        asprintf(&operands, "%s, %i", in.condition ? "TRUE" : "FALSE",
                 (i8)in.data);
        break;

    case S8:
        asprintf(&operands, "%i", (i8)in.data);
        break;

    case FLAG_A16:
        asprintf(&operands, "%s, " HEX, in.condition ? "TRUE" : "FALSE",
                 in.address);
        break;

    case A16:
        asprintf(&operands, HEX, in.address);
        break;

    case HL_REL:
        asprintf(&operands, "(HL)");
        break;

    case RST:
        asprintf(&operands, HEX, in.data);
        break;

    case R8_R8:
        asprintf(&operands, "%s, %s", register_names[in.reg1],
                 register_names[in.reg2]);
        break;

    case R8_D8:
        asprintf(&operands, "%s, " HEX8, register_names[in.reg1], in.data);
        break;

    case R8_HL_REL:
        asprintf(&operands, "%s, (HL) (=" HEX8 ")", register_names[in.reg1],
                 in.data);
        break;

    case A_R16_REL:
        asprintf(&operands, "A, " HEX8, in.data);
        break;

    case A_D16_REL:
        asprintf(&operands, "A, " HEX, in.data);
        break;

    case HL_REL_R8:
        asprintf(&operands, "(HL), %s", register_names[in.reg1]);
        break;

    case HL_REL_D8:
        asprintf(&operands, "(HL), " HEX8, in.data);
        break;

    case R16_REL_A:
        asprintf(&operands, HEX ", A", in.address);
        break;

    case D16_REL_A:
        asprintf(&operands, "(" HEX "), A", in.address);
        break;

    case R16_D16:
        asprintf(&operands, "%s, " HEX, register_names[in.reg1], in.data);
        break;

    case D16_REL_SP:
        asprintf(&operands, "(" HEX "), SP", in.data);
        break;

    case SP_HL:
        asprintf(&operands, "SP, HL");
        break;

    case HLI_A:
        asprintf(&operands, "(HL+), A");
        break;

    case HLD_A:
        asprintf(&operands, "(HL-), A");
        break;

    case A_HLI:
        asprintf(&operands, "A, (HL+)");
        break;

    case A_HLD:
        asprintf(&operands, "A, (HL-)");
        break;

    case A_C_REL:
        asprintf(&operands, "A, (C)");
        break;

    case A_D8_REL:
        asprintf(&operands, "A, " HEX8, in.data);
        break;

    case C_REL_A:
        asprintf(&operands, "(C), A");
        break;

    case D8_REL_A:
        asprintf(&operands, "(" HEX8 "), A", in.address);
        break;

    case A_R8:
        asprintf(&operands, "A, %s", register_names[in.reg2]);
        break;

    case A_D8:
        asprintf(&operands, "A, " HEX8, in.data);
        break;

    case A_HL_REL:
        asprintf(&operands, "A, (HL)");
        break;

    case NO_OPERAND:
    default:
        asprintf(&operands, "   ");
        break;
    }

// might be useful later ... we never know
#if 1
    char *registers = NULL;
    char *parameters = NULL;
    char *opcode = NULL;

    // print instruction's name
    asprintf(&opcode, "[" HEX "] %-4.4s ", in.pc,
             instruction_names[in.instruction]);

    // print 3 bytes at pc address: opcode + operands
    asprintf(&parameters, "(%02X %02X %02X) ", read_memory(in.pc),
             read_memory(in.pc + 1), read_memory(in.pc + 2));

    // print content of the registers
    asprintf(&registers, "AF=" HEX " BC=" HEX " DE=" HEX " HL=" HEX,
             read_register_16bit(REG_AF), read_register_16bit(REG_BC),
             read_register_16bit(REG_DE), read_register_16bit(REG_HL));

    asprintf(&line, "%s%-15.32s%s%s", opcode, operands, parameters, registers);

    free(opcode);
    free(operands);
    free(parameters);
    free(registers);
#else
    // print instruction's name
    asprintf(&line, "[" HEX "] %-4.4s %s \t(" HEX8 ")", in.pc,
             instruction_names[in.instruction], operands, read_memory(in.pc));
    free(operands);
#endif

    log_trace(line);
    free(line);
}
