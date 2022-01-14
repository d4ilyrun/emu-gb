#include <stdio.h>

#include "CPU/instruction.h"

static char *instruction_names[] = {
    "???",  "NOP", "JMP",  "LD",   "PUSH", "POP", "JP",  "JR",  "CALL", "RET",
    "RETI", "RST", "HALT", "STOP", "DI",   "EI",  "CCF", "SCF", "DAA",  "CPL",
    "ADD",  "ADC", "SUB",  "SBC",  "INC",  "DEC", "MUL", "AND", "OR",   "XOR"};

char *register_names[] = {
    "A", "F", "B", "C", "D", "E", "H", "L",
    "PC", "SP", "AF", "BC", "DE", "HL", "???"
};

// TODO: add nice colors :^)
static void display_instruction(u16 pc, struct instruction in)
{
    // print instruction's name
    printf("[0x%04X] %s    ", pc, instruction_names[in.instruction]);

    // TODO: print operands
    switch (in.type) {
    case R16:
    case R8:
        printf("%s", register_names[in.reg1]);
        break;
    case NO_OPERAND:
    default:
        break;
    }

    // print content of the registers
    printf("\tBC=0x%04X DE=0x%04X HL=0x%04X\n", read_register_16(REG_BC),
           read_register_16(REG_DE), read_register_16(REG_HL));
}

struct in_type {
    in_name name;
    operand_type type;
};

/*
 * List of all the instructions along with their type according to the
 * original Game Boy's opcode table.
 *
 * For a more detailed list:
 *  - https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
 */
struct in_type opcodes[] = {
    // First row: 0x0?
    [0x00] = {IN_NOP, NO_OPERAND},
};

/*
 * Fetch the instruction's data before execution.
 * Get the operand's types and values (if any).
 */
struct instruction fetch_instruction(u8 opcode)
{
    u16 pc                = read_register_16(REG_PC);
    struct in_type type   = opcodes[opcode];
    struct instruction in = {IN_ERR, ERR_OPERAND, REG_ERR, REG_ERR, 0xdead};

    in.instruction = type.name;
    in.type        = type.type;

    switch (in.type) {
    case NO_OPERAND:
    default:
        break;
    }

    display_instruction(pc - 1, in);

    return in;
}
