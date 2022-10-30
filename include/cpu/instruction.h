#pragma once

#include "cpu/cpu.h"
#include "cpu/timer.h"
#include "memory.h"
#include "utils/macro.h"
#include "utils/types.h"

/*
 * Run the next instruction at PC
 */
u8 execute_instruction();

__attribute__((unused)) static ALWAYS_INLINE u8 fetch_opcode()
{
    timer_tick();
    return read_memory(g_cpu.registers.pc++);
}

typedef enum instruction_name {
    IN_ADC,
    IN_ADD,
    IN_AND,
    IN_CALL,
    IN_CB,
    IN_CCF,
    IN_CP,
    IN_CPL,
    IN_DAA,
    IN_DEC,
    IN_DI,
    IN_EI,
    IN_ERR,
    IN_HALT,
    IN_INC,
    IN_JP,
    IN_JR,
    IN_LD,
    IN_LDH,
    IN_NOP,
    IN_OR,
    IN_POP,
    IN_PUSH,
    IN_RET,
    IN_RETI,
    IN_RLA,
    IN_RLCA,
    IN_RRA,
    IN_RRCA,
    IN_RST,
    IN_SBC,
    IN_SCF,
    IN_STOP,
    IN_SUB,
    IN_XOR,
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
 * - FLAG: condition ...
 *
 * Specific register:
 * - HL: HL register
 * - HLI/HLD: HL register with increment/decrement
 * - A: A register
 * - C: C register
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
    A16,
    HL_REL,
    S8,
    FLAG,
    RST, // special case: data in opcode
    D8,

    // Two operands with register as destination
    FLAG_A16,
    FLAG_S8,
    R8_R8,
    R8_D8,
    R8_HL_REL,
    A_R16_REL,
    A_D16_REL,
    R16_D16,
    SP_HL,
    SP_S8,
    A_HLD,
    A_HLI,
    A_C_REL,
    A_D8_REL,
    A_R8,
    A_D8,
    A_HL_REL,
    HL_R16,
    HL_S8,

    // other types with two operands
    HL_REL_R8,
    HL_REL_D8,
    R16_REL_A,
    D16_REL_A,
    D16_REL_SP,
    HLD_A,
    HLI_A,
    C_REL_A,
    D8_REL_A,
} operand_type;

#define IS_DST_REGISTER(_in) ((_in).type >= R8_R8 && (_in).type < HL_REL_R8)
#define IS_ONE_OPERAND(_in) (in.type < FLAG_A16)

#define HAS_CONDITION(_in) ((_in).type == FLAG_S8 || (_in).type == FLAG_A16)

/*
 * A struct representing an instruction and its operands.
 * To simply things we also include the instruction's cycle count and address.
 *
 * An instruction operand can be:
 * - 1 or 2 registers (16 or 8 bit)
 * - An immediate value (16 or 8 bit)
 * - An address (16-bit immediate value)
 * - A condition
 */
struct instruction {
    in_name instruction;
    operand_type type;
    u16 pc;

    // Operand values
    cpu_register_name reg1;
    cpu_register_name reg2;
    u16 address;
    bool condition;
    u16 data;

    // Cycle count
    u8 cycle_count;
    u8 cycle_count_false; // For conditional jumps
};

/*
 * (MSB -> LSB)
 * x = the opcode's 1st octal digit (i.e. bits 7-6)
 * y = the opcode's 2nd octal digit (i.e. bits 5-3)
 * z = the opcode's 3rd octal digit (i.e. bits 2-0)
 * p = y rightshifted one position (i.e. bits 5-4)
 * q = y modulo 2 (i.e. bit 3)
 */

#define OPCODE_X(_opcode) ((_opcode) >> 6)
#define OPCODE_Y(_opcode) (((_opcode) >> 3) & 0x07)
#define OPCODE_Z(_opcode) ((_opcode)&0x07)
#define OPCODE_Q(_opcode) (((_opcode) >> 3) & 0x01)
#define OPCODE_P(_opcode) (((_opcode) >> 4) & 0x03)
#define OPCODE_FLAG(_opcode) (OPCODE_Y(_opcode) & 0x03)

struct instruction fetch_instruction(u8 opcode);
void display_instruction(struct instruction in);

/*
 * The following part of the file is centered around the CB prefixed
 * instructions. CB prefixed instructions are instructions with a 2 bytes long
 * opcode starting with CB.
 */

typedef enum {
    CB_ROT,
    CB_TEST,
    CB_RES,
    CB_SET,
} cb_instruction_type;

struct cb_instruction {
    cb_instruction_type type;

    union {
        u8 bit;
        u8 rot_type;
    };

    u8 data;
    u16 address;
    bool is_address;
    cpu_register_name reg;
};

u8 cb_execute_instruction();
