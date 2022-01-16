#include "CPU/flag.h"
#include "CPU/instruction.h"
#include "CPU/memory.h"

struct in_type {
    in_name name;
    operand_type type;
    u8 cycle_count;
    u8 cycle_count_false; // For conditional jumps
};

static u16 read_16bit_data()
{
    cpu.registers.pc += 2;
    return read_memory_16bit(cpu.registers.pc - 2);
}

static u8 read_8bit_data()
{
    return read_memory(cpu.registers.pc++);
}

/*
 * Read condition from operand:
 *
 * 000 = NZ
 * 001 = Z
 * 010 = N3
 * 011 = C
 */
static bool read_flag(u8 opcode)
{
    switch (OPCODE_Y(opcode)) {
    case 0x0:
        return !get_flag(FLAG_Z);
    case 0x1:
        return get_flag(FLAG_Z);
    case 0x2:
        return !get_flag(FLAG_C);
    case 0x3:
        return get_flag(FLAG_C);
    default:
        return true;
    }
}

/*
 * List of all the instructions along with their type according to the
 * original Game Boy's opcode table.
 *
 * For a more detailed list:
 *  - https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html
 */
struct in_type opcodes[] = {
    // First row: 0x0
    [0x00] = {IN_NOP, NO_OPERAND, 1},

    // 0x1
    [0x18] = {IN_JR, S8, 3},

    // 0x2
    [0x20] = {IN_JR, FLAG_S8, 3, 2},
    [0x28] = {IN_JR, FLAG_S8, 3, 2},

    // 0x3
    [0x30] = {IN_JR, FLAG_S8, 3, 2},
    [0x38] = {IN_JR, FLAG_S8, 3, 2},

    // 0xC
    [0xC0] = {IN_RET, FLAG, 5, 2},
    [0xC2] = {IN_JP, FLAG_A16, 4, 3},
    [0xC3] = {IN_JP, A16, 4},
    [0xC4] = {IN_CALL, FLAG_A16, 6, 3},
    [0xC7] = {IN_RST, RST, 4},
    [0xC8] = {IN_RET, FLAG, 5, 2},
    [0xC9] = {IN_RET, NO_OPERAND, 4},
    [0xCA] = {IN_JP, FLAG_A16, 4, 3},
    [0xCC] = {IN_CALL, FLAG_A16, 6, 3},
    [0xCD] = {IN_CALL, A16, 6},
    [0xCF] = {IN_RST, RST, 4},

    // OxD
    [0xD0] = {IN_RET, FLAG, 5, 2},
    [0xD2] = {IN_JP, FLAG_A16, 4, 3},
    [0xD4] = {IN_CALL, FLAG_A16, 6, 3},
    [0xD7] = {IN_RST, RST, 4},
    [0xD8] = {IN_RET, FLAG, 5, 2},
    [0xD9] = {IN_RETI, NO_OPERAND, 4},
    [0xDA] = {IN_JP, FLAG_A16, 4, 3},
    [0xDC] = {IN_CALL, FLAG_A16, 6, 3},
    [0xDF] = {IN_RST, RST, 4},

    // 0xE
    [0xE7] = {IN_RST, RST, 4},
    [0xE9] = {IN_JP, HL_IMM, 1},
    [0xEF] = {IN_RST, RST, 4},

    // 0xF
    [0xF7] = {IN_RST, RST, 4},
    [0xFF] = {IN_RST, RST, 4},

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

    in.instruction       = type.name;
    in.type              = type.type;
    in.pc                = pc - 1;
    in.condition         = true;
    in.cycle_count       = type.cycle_count;
    in.cycle_count_false = type.cycle_count_false;

    switch (in.type) {
    case R8:
        // TODO
        break;

    case R16:
        // TODO
        break;

    case A16:
        in.address = read_16bit_data();
        break;

    case HL_IMM:
        in.address = read_register_16(REG_HL);
        break;

    case FLAG_A16:
        in.condition = read_flag(opcode);
        in.address   = read_16bit_data();
        break;

    case S8:
        in.data = read_8bit_data();
        break;

    case FLAG_S8:
        in.condition = read_flag(opcode);
        in.data      = read_8bit_data();
        break;

    case FLAG:
        in.condition = read_flag(opcode);
        break;

    case RST:
        in.data = OPCODE_Y(opcode);
        break;

    case NO_OPERAND:
    default:
        break;
    }

    display_instruction(in);

    return in;
}
