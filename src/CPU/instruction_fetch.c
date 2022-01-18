#include <err.h>

#include "CPU/flag.h"
#include "CPU/instruction.h"
#include "CPU/memory.h"
#include "utils/macro.h"

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
 * Find which 8-bit register an operand points to from the opcode's value.
 *
 * Register  Operand
 *   A        111
 *   B        000
 *   C        001
 *   D        010
 *   E        011
 *   H        100
 *   L        101
 */
static cpu_register_name find_register(uint8_t code)
{
    if (code == 0x7)
        return REG_A;

    // TODO: assert not reached macro
    if (code == 0x6)
        errx(1, "Unreachable code reached: %s", __FUNCTION__);

    return REG_B + code;
}

/*
 * Find which 16-bit register an operand points to from the opcode's value.
 *
 * Register  Operand
 *   BC        00
 *   DE        01
 *   HL        10
 *   SP        11
 */
static cpu_register_name find_register_16bit(uint8_t code)
{
    if (code == 0x3)
        return REG_SP;

    return REG_BC + code;
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
    [0x01] = {IN_LD, R16_D16, 3},
    [0x02] = {IN_LD, R16_REL_A, 2},
    [0x03] = {IN_INC, R16, 2},
    [0x04] = {IN_INC, R8, 2},
    [0x05] = {IN_DEC, R8, 2},
    [0x06] = {IN_LD, R8_D8, 2},
    [0x07] = {IN_RLCA, NO_OPERAND, 1},
    [0x08] = {IN_LD, D16_REL_SP, 5},
    [0x09] = {IN_ADD, HL_R16, 2},
    [0x0A] = {IN_LD, A_R16_REL, 2},
    [0x0B] = {IN_DEC, R16, 2},
    [0x0C] = {IN_INC, R8, 2},
    [0x0D] = {IN_DEC, R8, 2},
    [0x0E] = {IN_LD, R8_D8, 2},
    [0x0F] = {IN_RRCA, NO_OPERAND, 1},

    // 0x1
    [0x11] = {IN_LD, R16_D16, 3},
    [0x12] = {IN_LD, R16_REL_A, 2},
    [0x13] = {IN_INC, R16, 2},
    [0x14] = {IN_INC, R8, 2},
    [0x15] = {IN_DEC, R8, 2},
    [0x16] = {IN_LD, R8_D8, 2},
    [0x17] = {IN_RLA, NO_OPERAND, 1},
    [0x18] = {IN_JR, S8, 3},
    [0x19] = {IN_ADD, HL_R16, 2},
    [0x1A] = {IN_LD, A_R16_REL, 2},
    [0x1B] = {IN_DEC, R16, 2},
    [0x1C] = {IN_INC, R8, 2},
    [0x1D] = {IN_DEC, R8, 2},
    [0x1E] = {IN_LD, R8_D8, 2},
    [0x1F] = {IN_RRA, NO_OPERAND, 1},

    // 0x2
    [0x20] = {IN_JR, FLAG_S8, 3, 2},
    [0x21] = {IN_LD, R16_D16, 3},
    [0x22] = {IN_LD, HLI_A, 2},
    [0x23] = {IN_INC, R16, 2},
    [0x24] = {IN_INC, R8, 2},
    [0x25] = {IN_DEC, R8, 2},
    [0x26] = {IN_LD, R8_D8, 2},
    [0x27] = {IN_DAA, NO_OPERAND, 1},
    [0x28] = {IN_JR, FLAG_S8, 3, 2},
    [0x29] = {IN_ADD, HL_R16, 2},
    [0x2A] = {IN_LD, A_HLD, 2},
    [0x2B] = {IN_DEC, R16, 2},
    [0x2C] = {IN_INC, R8, 2},
    [0x2D] = {IN_DEC, R8, 2},
    [0x2E] = {IN_LD, R8_D8, 2},
    [0x2F] = {IN_CPL, NO_OPERAND, 1},

    // 0x3
    [0x30] = {IN_JR, FLAG_S8, 3, 2},
    [0x31] = {IN_LD, R16_D16, 3},
    [0x32] = {IN_LD, HLD_A, 2},
    [0x33] = {IN_INC, R16, 2},
    [0x34] = {IN_INC, HL_REL, 2},
    [0x35] = {IN_DEC, HL_REL, 2},
    [0x36] = {IN_LD, HL_REL_D8, 3},
    [0x37] = {IN_SCF, NO_OPERAND, 1},
    [0x38] = {IN_JR, FLAG_S8, 3, 2},
    [0x39] = {IN_ADD, HL_R16, 2},
    [0x3A] = {IN_LD, A_HLD, 2},
    [0x3B] = {IN_DEC, R16, 2},
    [0x3C] = {IN_INC, R8, 2},
    [0x3D] = {IN_DEC, R8, 2},
    [0x3E] = {IN_LD, R8_D8, 2},
    [0x3F] = {IN_CCF, NO_OPERAND, 1},

    // 0x4
    [0x40] = {IN_LD, R8_R8, 1},
    [0x41] = {IN_LD, R8_R8, 1},
    [0x42] = {IN_LD, R8_R8, 1},
    [0x43] = {IN_LD, R8_R8, 1},
    [0x44] = {IN_LD, R8_R8, 1},
    [0x45] = {IN_LD, R8_R8, 1},
    [0x46] = {IN_LD, R8_HL_REL, 2},
    [0x47] = {IN_LD, R8_R8, 1},
    [0x48] = {IN_LD, R8_R8, 1},
    [0x49] = {IN_LD, R8_R8, 1},
    [0x4A] = {IN_LD, R8_R8, 1},
    [0x4B] = {IN_LD, R8_R8, 1},
    [0x4C] = {IN_LD, R8_R8, 1},
    [0x4D] = {IN_LD, R8_R8, 1},
    [0x4E] = {IN_LD, R8_HL_REL, 2},
    [0x4F] = {IN_LD, R8_R8, 1},

    // 0x5
    [0x50] = {IN_LD, R8_R8, 1},
    [0x51] = {IN_LD, R8_R8, 1},
    [0x52] = {IN_LD, R8_R8, 1},
    [0x53] = {IN_LD, R8_R8, 1},
    [0x54] = {IN_LD, R8_R8, 1},
    [0x55] = {IN_LD, R8_R8, 1},
    [0x56] = {IN_LD, R8_HL_REL, 2},
    [0x57] = {IN_LD, R8_R8, 1},
    [0x58] = {IN_LD, R8_R8, 1},
    [0x59] = {IN_LD, R8_R8, 1},
    [0x5A] = {IN_LD, R8_R8, 1},
    [0x5B] = {IN_LD, R8_R8, 1},
    [0x5C] = {IN_LD, R8_R8, 1},
    [0x5D] = {IN_LD, R8_R8, 1},
    [0x5E] = {IN_LD, R8_HL_REL, 2},
    [0x5F] = {IN_LD, R8_R8, 1},

    // 0x6
    [0x60] = {IN_LD, R8_R8, 1},
    [0x61] = {IN_LD, R8_R8, 1},
    [0x62] = {IN_LD, R8_R8, 1},
    [0x63] = {IN_LD, R8_R8, 1},
    [0x64] = {IN_LD, R8_R8, 1},
    [0x65] = {IN_LD, R8_R8, 1},
    [0x66] = {IN_LD, R8_HL_REL, 2},
    [0x67] = {IN_LD, R8_R8, 1},
    [0x68] = {IN_LD, R8_R8, 1},
    [0x69] = {IN_LD, R8_R8, 1},
    [0x6A] = {IN_LD, R8_R8, 1},
    [0x6B] = {IN_LD, R8_R8, 1},
    [0x6C] = {IN_LD, R8_R8, 1},
    [0x6D] = {IN_LD, R8_R8, 1},
    [0x6E] = {IN_LD, R8_HL_REL, 2},
    [0x6F] = {IN_LD, R8_R8, 1},

    // 0x7
    [0x70] = {IN_LD, HL_REL_R8, 2},
    [0x71] = {IN_LD, HL_REL_R8, 2},
    [0x72] = {IN_LD, HL_REL_R8, 2},
    [0x73] = {IN_LD, HL_REL_R8, 2},
    [0x74] = {IN_LD, HL_REL_R8, 2},
    [0x75] = {IN_LD, HL_REL_R8, 2},
    [0x76] = {IN_HALT, NO_OPERAND, 1},
    [0x77] = {IN_LD, HL_REL_R8, 2},
    [0x78] = {IN_LD, R8_R8, 1},
    [0x79] = {IN_LD, R8_R8, 1},
    [0x7A] = {IN_LD, R8_R8, 1},
    [0x7B] = {IN_LD, R8_R8, 1},
    [0x7C] = {IN_LD, R8_R8, 1},
    [0x7D] = {IN_LD, R8_R8, 1},
    [0x7E] = {IN_LD, R8_HL_REL, 2},
    [0x7F] = {IN_LD, R8_R8, 1},

    // 0x8
    [0x80] = {IN_ADD, A_R8, 1},
    [0x81] = {IN_ADD, A_R8, 1},
    [0x82] = {IN_ADD, A_R8, 1},
    [0x83] = {IN_ADD, A_R8, 1},
    [0x84] = {IN_ADD, A_R8, 1},
    [0x85] = {IN_ADD, A_R8, 1},
    [0x86] = {IN_ADD, A_HL_REL, 2},
    [0x87] = {IN_ADD, A_R8, 1},
    [0x88] = {IN_ADC, A_R8, 1},
    [0x89] = {IN_ADC, A_R8, 1},
    [0x8A] = {IN_ADC, A_R8, 1},
    [0x8B] = {IN_ADC, A_R8, 1},
    [0x8C] = {IN_ADC, A_R8, 1},
    [0x8D] = {IN_ADC, A_R8, 1},
    [0x8E] = {IN_ADC, A_HL_REL, 2},
    [0x8F] = {IN_ADC, A_R8, 1},

    // 0x9
    [0x90] = {IN_SUB, A_R8, 1},
    [0x91] = {IN_SUB, A_R8, 1},
    [0x92] = {IN_SUB, A_R8, 1},
    [0x93] = {IN_SUB, A_R8, 1},
    [0x94] = {IN_SUB, A_R8, 1},
    [0x95] = {IN_SUB, A_R8, 1},
    [0x96] = {IN_SUB, A_HL_REL, 2},
    [0x97] = {IN_SUB, A_R8, 1},
    [0x98] = {IN_SBC, A_R8, 1},
    [0x99] = {IN_SBC, A_R8, 1},
    [0x9A] = {IN_SBC, A_R8, 1},
    [0x9B] = {IN_SBC, A_R8, 1},
    [0x9C] = {IN_SBC, A_R8, 1},
    [0x9D] = {IN_SBC, A_R8, 1},
    [0x9E] = {IN_SBC, A_HL_REL, 2},
    [0x9F] = {IN_SBC, A_R8, 1},

    // 0xA
    [0xA0] = {IN_AND, A_R8, 1},
    [0xA1] = {IN_AND, A_R8, 1},
    [0xA2] = {IN_AND, A_R8, 1},
    [0xA3] = {IN_AND, A_R8, 1},
    [0xA4] = {IN_AND, A_R8, 1},
    [0xA5] = {IN_AND, A_R8, 1},
    [0xA6] = {IN_AND, A_HL_REL, 2},
    [0xA7] = {IN_AND, A_R8, 1},
    [0xA8] = {IN_XOR, A_R8, 1},
    [0xA9] = {IN_XOR, A_R8, 1},
    [0xAA] = {IN_XOR, A_R8, 1},
    [0xAB] = {IN_XOR, A_R8, 1},
    [0xAC] = {IN_XOR, A_R8, 1},
    [0xAD] = {IN_XOR, A_R8, 1},
    [0xAE] = {IN_XOR, A_HL_REL, 2},
    [0xAF] = {IN_XOR, A_R8, 1},

    // 0xB
    [0xB0] = {IN_OR, A_R8, 1},
    [0xB1] = {IN_OR, A_R8, 1},
    [0xB2] = {IN_OR, A_R8, 1},
    [0xB3] = {IN_OR, A_R8, 1},
    [0xB4] = {IN_OR, A_R8, 1},
    [0xB5] = {IN_OR, A_R8, 1},
    [0xB6] = {IN_OR, A_HL_REL, 2},
    [0xB7] = {IN_OR, A_R8, 1},
    [0xB8] = {IN_CP, A_R8, 1},
    [0xB9] = {IN_CP, A_R8, 1},
    [0xBA] = {IN_CP, A_R8, 1},
    [0xBB] = {IN_CP, A_R8, 1},
    [0xBC] = {IN_CP, A_R8, 1},
    [0xBD] = {IN_CP, A_R8, 1},
    [0xBE] = {IN_CP, A_HL_REL, 2},
    [0xBF] = {IN_CP, A_R8, 1},

    // 0xC
    [0xC0] = {IN_RET, FLAG, 5, 2},
    [0xC1] = {IN_POP, R16, 4},
    [0xC2] = {IN_JP, FLAG_A16, 4, 3},
    [0xC3] = {IN_JP, A16, 4},
    [0xC4] = {IN_CALL, FLAG_A16, 6, 3},
    [0xC5] = {IN_PUSH, R16, 4},
    [0xC6] = {IN_ADD, A_D8, 2},
    [0xC8] = {IN_RET, FLAG, 5, 2},
    [0xC9] = {IN_RET, NO_OPERAND, 4},
    [0xCA] = {IN_JP, FLAG_A16, 4, 3},
    [0xCC] = {IN_CALL, FLAG_A16, 6, 3},
    [0xCD] = {IN_CALL, A16, 6},
    [0xCE] = {IN_ADC, A_D8, 2},
    [0xCF] = {IN_RST, RST, 4},

    // OxD
    [0xD0] = {IN_RET, FLAG, 5, 2},
    [0xD1] = {IN_POP, R16, 4},
    [0xD2] = {IN_JP, FLAG_A16, 4, 3},
    [0xD4] = {IN_CALL, FLAG_A16, 6, 3},
    [0xD5] = {IN_PUSH, R16, 4},
    [0xD6] = {IN_SUB, A_D8, 2},
    [0xD7] = {IN_RST, RST, 4},
    [0xD8] = {IN_RET, FLAG, 5, 2},
    [0xD9] = {IN_RETI, NO_OPERAND, 4},
    [0xDA] = {IN_JP, FLAG_A16, 4, 3},
    [0xDC] = {IN_CALL, FLAG_A16, 6, 3},
    [0xDE] = {IN_SBC, A_D8, 2},
    [0xDF] = {IN_RST, RST, 4},

    // 0xE
    [0xE0] = {IN_LDH, D8_REL_A, 3},
    [0xE1] = {IN_POP, R16, 4},
    [0xE2] = {IN_LDH, C_REL_A, 2},
    [0xE5] = {IN_PUSH, R16, 4},
    [0xE6] = {IN_AND, A_D8, 2},
    [0xE7] = {IN_RST, RST, 4},
    [0xE8] = {IN_ADD, SP_S8, 4},
    [0xE9] = {IN_JP, HL_REL, 1},
    [0xEA] = {IN_LD, D16_REL_A, 4},
    [0xEE] = {IN_XOR, A_D8, 2},
    [0xEF] = {IN_RST, RST, 4},

    // 0xF
    [0xF0] = {IN_LDH, A_D8_REL, 3},
    [0xF1] = {IN_POP, R16, 4},
    [0xF2] = {IN_LDH, A_C_REL, 2},
    [0xF3] = {IN_DI, NO_OPERAND, 1},
    [0xF5] = {IN_PUSH, R16, 4},
    [0xF6] = {IN_OR, A_D8, 2},
    [0xF7] = {IN_RST, RST, 4},
    [0xF9] = {IN_LD, SP_HL, 2},
    [0xFA] = {IN_LD, A_D16_REL, 4},
    [0xFB] = {IN_EI, NO_OPERAND, 1},
    [0xFE] = {IN_CP, A_D8, 2},
    [0xFF] = {IN_RST, RST, 4},

};

/*
 * Fetch the instruction's data before execution.
 * Get the operand's types and values (if any).
 */
struct instruction fetch_instruction(u8 opcode)
{
    u16 pc = read_register_16bit(REG_PC);
    struct in_type type = opcodes[opcode];
    struct instruction in = {IN_ERR, ERR_OPERAND, REG_ERR, REG_ERR, 0xdead};

    in.instruction = type.name;
    in.type = type.type;
    in.pc = pc - 1;
    in.condition = true;
    in.cycle_count = type.cycle_count;
    in.cycle_count_false = type.cycle_count_false;

    switch (in.type) {

#pragma region one_operand

    case R8:
        in.reg1 = find_register(OPCODE_Y(opcode));
        break;

    case R16:
        in.reg1 = find_register_16bit(OPCODE_P(opcode));
        break;

    case A16:
        in.address = read_16bit_data();
        break;

    case HL_REL:
        in.address = read_register_16bit(REG_HL);
        break;

    case FLAG_A16:
        in.condition = read_flag(opcode);
        in.address = read_16bit_data();
        break;

    case S8:
        in.data = read_8bit_data();
        break;

    case FLAG_S8:
        in.condition = read_flag(opcode);
        in.data = read_8bit_data();
        break;

    case FLAG:
        in.condition = read_flag(opcode);
        break;

    case RST:
        in.data = OPCODE_Y(opcode);
        break;

    case D8:
        in.data = read_8bit_data();
        break;

#pragma endregion one_operand

#pragma region two_operand_dst_register

    case R8_R8:
        in.reg1 = find_register(OPCODE_Y(opcode));
        in.reg2 = find_register(OPCODE_Z(opcode));
        break;

    case R8_D8:
        in.reg1 = find_register(OPCODE_Y(opcode));
        in.data = read_8bit_data();
        break;

    case R8_HL_REL:
        in.reg1 = find_register(OPCODE_Y(opcode));
        in.data = read_memory(read_register_16bit(REG_HL));
        break;

    case A_R16_REL:
        in.reg1 = REG_A;
        in.data = read_memory_16bit(
            read_register_16bit(find_register_16bit(OPCODE_P(opcode))));
        break;

    case A_D16_REL:
        in.reg1 = REG_A;
        in.data = read_memory_16bit(read_16bit_data());
        break;

    case R16_D16:
        in.reg1 = find_register_16bit(OPCODE_P(opcode));
        in.data = read_16bit_data();
        break;

    case SP_HL:
        in.reg1 = REG_SP;
        in.reg2 = REG_HL;
        break;

    case A_HLD:
        in.reg1 = REG_A;
        in.data = read_memory(read_register_16bit(REG_HL));
        write_register_16bit(REG_HL, read_register_16bit(REG_HL) - 1);
        break;

    case A_HLI:
        in.reg1 = REG_A;
        in.data = read_memory(read_register_16bit(REG_HL));
        write_register_16bit(REG_HL, read_register_16bit(REG_HL) + 1);
        break;

    case A_C_REL:
        in.reg1 = REG_A;
        in.data = read_memory(read_register(REG_C) + 0xFF00);
        break;

    case A_D8_REL:
        in.reg1 = REG_A;
        in.data = read_memory(read_8bit_data() + 0xFF00);
        break;

    case A_R8:
        in.reg1 = REG_A;
        in.reg2 = find_register(OPCODE_Z(opcode));
        break;

    case A_D8:
        in.reg1 = REG_A;
        in.reg2 = read_8bit_data();
        break;

    case A_HL_REL:
        in.reg1 = REG_A;
        in.data = read_memory(read_register_16bit(REG_HL));
        break;

    case HL_R16:
        in.reg1 = REG_HL;
        in.reg2 = find_register_16bit(OPCODE_P(opcode));
        break;

    case SP_S8:
        in.reg1 = REG_SP;
        in.data = read_8bit_data();
        break;

#pragma endregion two_operand_dst_register

#pragma region two_operand_dst_address

    case HL_REL_R8:
        in.address = read_memory(read_register_16bit(REG_HL));
        in.reg1 = find_register(OPCODE_Z(opcode));
        break;

    case HL_REL_D8:
        in.address = read_memory(read_register_16bit(REG_HL));
        in.data = read_8bit_data();
        break;

    case R16_REL_A:
        in.address = read_register_16bit(find_register_16bit(OPCODE_P(opcode)));
        in.reg1 = REG_A;
        break;

    case D16_REL_A:
        in.address = read_16bit_data();
        in.reg1 = REG_A;
        break;

    case D16_REL_SP:
        in.address = read_16bit_data();
        in.reg1 = read_register_16bit(REG_SP);
        break;

    case HLD_A:
        in.address = read_memory(read_register_16bit(REG_HL));
        in.reg1 = REG_A;
        write_register_16bit(REG_HL, read_register_16bit(REG_HL) - 1);
        break;

    case HLI_A:
        in.address = read_memory(read_register_16bit(REG_HL));
        in.reg1 = REG_A;
        write_register_16bit(REG_HL, read_register_16bit(REG_HL) + 1);
        break;

    case C_REL_A:
        in.address = read_register(REG_C) + 0xFF00;
        in.reg1 = REG_A;
        break;

    case D8_REL_A:
        in.address = read_8bit_data() + 0xFF00;
        in.reg1 = REG_A;
        break;

#pragma endregion two_operand_dst_address

    case NO_OPERAND:
    default:
        break;
    }

    display_instruction(in);

    return in;
}
