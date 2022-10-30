#define _GNU_SOURCE // needed for asprintf

#include <stdio.h>
#include <stdlib.h>

#include "cpu/instruction.h"
#include "options.h"
#include "utils/log.h"
#include "utils/macro.h"

#ifdef TEST_ROM
#include "test_rom.h"
#endif

static char *g_instruction_names[] = {
    "ADC", "ADD",  "AND", "CALL", "CB",   "CCF",  "CP",   "CPL", "DAA",
    "DEC", "DI",   "EI",  "ERR",  "HALT", "INC",  "JP",   "JR",  "LD",
    "LDH", "NOP",  "OR",  "POP",  "PUSH", "RET",  "RETI", "RLA", "RLCA",
    "RRA", "RRCA", "RST", "SBC",  "SCF",  "STOP", "SUB",  "XOR",
};

char *g_condition_names[] = {"NZ", "Z", "NC", "C", "???"};

char *g_register_names[] = {"A",  "F",  "B",  "C",  "D",  "E",  "H",  "L",
                            "PC", "SP", "AF", "BC", "DE", "HL", "???"};

void display_instruction(struct instruction in)
{
    if (get_options()->trace == false)
        return;

    char *line_ptr = NULL;
    char *operands_ptr = NULL;

    switch (in.type) {
    case R16:
    case R8:
        asprintf(&operands_ptr, "%s", g_register_names[in.reg1]);
        break;

    case FLAG:
        asprintf(&operands_ptr, "%s, ", in.condition ? "TRUE" : "FALSE");
        break;

    case FLAG_S8:
        asprintf(&operands_ptr, "%s, %i", in.condition ? "TRUE" : "FALSE",
                 (i8)in.data);
        break;

    case S8:
        asprintf(&operands_ptr, "%i", (i8)in.data);
        break;

    case FLAG_A16:
        asprintf(&operands_ptr, "%s, " HEX, in.condition ? "TRUE" : "FALSE",
                 in.address);
        break;

    case A16:
        asprintf(&operands_ptr, HEX, in.address);
        break;

    case HL_REL:
        asprintf(&operands_ptr, "(HL)");
        break;

    case RST:
        asprintf(&operands_ptr, HEX, in.data);
        break;

    case R8_R8:
        asprintf(&operands_ptr, "%s, %s", g_register_names[in.reg1],
                 g_register_names[in.reg2]);
        break;

    case R8_D8:
        asprintf(&operands_ptr, "%s, " HEX8, g_register_names[in.reg1],
                 in.data);
        break;

    case R8_HL_REL:
        asprintf(&operands_ptr, "%s, (HL) (=" HEX8 ")",
                 g_register_names[in.reg1], in.data);
        break;

    case A_R16_REL:
        asprintf(&operands_ptr, "A, " HEX8, in.data);
        break;

    case A_D16_REL:
        asprintf(&operands_ptr, "A, " HEX, in.data);
        break;

    case HL_REL_R8:
        asprintf(&operands_ptr, "(HL), %s", g_register_names[in.reg1]);
        break;

    case HL_REL_D8:
        asprintf(&operands_ptr, "(HL), " HEX8, in.data);
        break;

    case R16_REL_A:
        asprintf(&operands_ptr, HEX ", A", in.address);
        break;

    case D16_REL_A:
        asprintf(&operands_ptr, "(" HEX "), A", in.address);
        break;

    case R16_D16:
        asprintf(&operands_ptr, "%s, " HEX, g_register_names[in.reg1], in.data);
        break;

    case D16_REL_SP:
        asprintf(&operands_ptr, "(" HEX "), SP", in.data);
        break;

    case SP_HL:
        asprintf(&operands_ptr, "SP, HL");
        break;

    case HLI_A:
        asprintf(&operands_ptr, "(HL+), A");
        break;

    case HLD_A:
        asprintf(&operands_ptr, "(HL-), A");
        break;

    case A_HLI:
        asprintf(&operands_ptr, "A, (HL+)");
        break;

    case A_HLD:
        asprintf(&operands_ptr, "A, (HL-)");
        break;

    case A_C_REL:
        asprintf(&operands_ptr, "A, (C)");
        break;

    case A_D8_REL:
        asprintf(&operands_ptr, "A, " HEX8, in.data);
        break;

    case C_REL_A:
        asprintf(&operands_ptr, "(C), A");
        break;

    case D8_REL_A:
        asprintf(&operands_ptr, "(" HEX8 "), A", in.address);
        break;

    case A_R8:
        asprintf(&operands_ptr, "A, %s", g_register_names[in.reg2]);
        break;

    case A_D8:
        asprintf(&operands_ptr, "A, " HEX8, in.data);
        break;

    case A_HL_REL:
        asprintf(&operands_ptr, "A, (HL)");
        break;

    case NO_OPERAND:
    default:
        asprintf(&operands_ptr, "   ");
        break;
    }

// might be useful later ... we never know
#if 1
    char *registers_ptr = NULL;
    char *parameters_ptr = NULL;
    char *opcode_ptr = NULL;

    // print instruction's name
    asprintf(&opcode_ptr, "[" HEX "] %-4.4s ", in.pc,
             g_instruction_names[in.instruction]);

    // print 3 bytes at pc address: opcode + operands
    asprintf(&parameters_ptr, "(%02X %02X %02X) ", read_memory(in.pc),
             read_memory(in.pc + 1), read_memory(in.pc + 2));

    // print content of the registers
    asprintf(&registers_ptr, "AF=" HEX " BC=" HEX " DE=" HEX " HL=" HEX,
             read_register_16bit(REG_AF), read_register_16bit(REG_BC),
             read_register_16bit(REG_DE), read_register_16bit(REG_HL));

    asprintf(&line_ptr, "%s%-15.32s%s%s", opcode_ptr, operands_ptr,
             parameters_ptr, registers_ptr);

    free(opcode_ptr);
    free(operands_ptr);
    free(parameters_ptr);
    free(registers_ptr);
#else
    // print instruction's name
    asprintf(&line, "[" HEX "] %-4.4s %s \t(" HEX8 ")", in.pc,
             instruction_names[in.instruction], operands, read_memory(in.pc));
    free(operands);
#endif

    log_trace(line_ptr);
    free(line_ptr);
}
