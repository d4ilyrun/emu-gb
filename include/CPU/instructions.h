#pragma once

#include "cpu.h"
#include "utils/types.h"
#include "memory.h"

/*
 * Run the next instruction at PC
 */
u8 execute_instruction();

typedef enum instruction_name {
    ERR_IN,
    NOP,
    JMP,
    LD,
    PUSH,
    POP,
    JP,
    JR,
    CALL,
    RET,
    RETI,
    RST,
    HALT,
    STOP,
    DI,
    EI,
    CCF,
    SCF,
    DAA,
    CPL,
    ADD,
    ADC,
    SUB,
    SBC,
    INC,
    DEC,
    MUL,
    AND,
    OR,
    XOR,
} in_name;

/*
 * Different types of operands possible for an instruction.
 *
 * Meaning:
 * - R8/16: 8/16-bit register
 * - D8/D16: 16-bit immediate data
 * - A16: 16-bit address
 * - U8: 8-bit unsigned data, which are added to $FF00 in certain instructions
 * - S8: 8-bit signed data, which are added to PC
 * - FLAG: flag ...
 *
 * Prefixes:
 * - REL: relative addressing
 */
typedef enum operand_type {
    ERR_OPERAND,
    NO_OPERAND,

    // Only one operand
    R8,
    R16,

    // Two operands
} operand_type;

/*
 * A struct representing an instruction and its operands.
 * To simply things we also include the instruction's cycle count.
 *
 * An instruction operand can be:
 * - 1 or 2 registers (16 or 8 bit)
 * - An immediate value (16 or 8 bit)
 * - An address (16-bit immediate value)
 * - A flag
 */
struct instruction {
    in_name instruction;
    operand_type type;
    cpu_register_name reg1;
    cpu_register_name reg2;
    // FLAG
    u16 data;
    u8 cycle_count;
};

inline u8 fetch_opcode()
{
    return read_memory(cpu.registers.pc++);
}

struct instruction fetch_instruction(u8 opcode);
