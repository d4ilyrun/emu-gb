#include "CPU/instruction.h"

#include <stdlib.h>

#include "CPU/flag.h"
#include "CPU/interrupt.h"
#include "CPU/stack.h"
#include "options.h"
#include "utils/error.h"
#include "utils/log.h"

#define INSTRUCTION(_name) static u8 _name(struct instruction in)

typedef u8 (*in_handler)(struct instruction);

INSTRUCTION(invalid)
{
    fatal_error("\nInvalid instruction. (code: " HEX8 ")\n",
                read_memory(in.pc));
    exit(-1);
}

INSTRUCTION(nop)
{
    return in.cycle_count;
}

INSTRUCTION(jp)
{
    if (!in.condition)
        return in.cycle_count_false;
    write_register_16bit(REG_PC, in.address);
    return in.cycle_count;
}

INSTRUCTION(jr)
{
    if (get_options()->exit_infinite_loop && (i8)in.data == -2) {
        fatal_error("Infinite JR loop");
    }

    if (!in.condition)
        return in.cycle_count_false;

    write_register_16bit(REG_PC, read_register_16bit(REG_PC) + (i8)in.data);
    return in.cycle_count_false;
}

INSTRUCTION(call)
{
    if (!in.condition)
        return in.cycle_count_false;
    stack_push_16bit(read_register_16bit(REG_PC));
    write_register_16bit(REG_PC, in.address);
    return in.cycle_count;
}

INSTRUCTION(ret)
{
    if (!in.condition)
        return in.cycle_count_false;
    write_register_16bit(REG_PC, stack_pop_16bit());
    return in.cycle_count;
}

INSTRUCTION(reti)
{
    cpu.registers.pc = stack_pop_16bit();
    interrupt_set_ime(true);
    return in.cycle_count;
}

INSTRUCTION(rst)
{
    stack_push_16bit(read_register_16bit(REG_PC));
    write_register_16bit(REG_PC, in.data);
    return in.cycle_count;
}

// for 0xF8
INSTRUCTION(add);

INSTRUCTION(ld)
{
    if (in.type == HL_S8) {
        u16 val = read_register_16bit(REG_SP);
        i8 data = in.data;

        write_register_16bit(REG_HL, val + data);

        set_all_flags(0, 0, 0, 0);
        set_flag(FLAG_C, ((val & 0xFF) + (data & 0xFF)) & 0x100);
        set_flag(FLAG_H, ((val & 0xF) + (data & 0xF)) & 0x10);

        return in.cycle_count;
    }

    switch (in.type) {
    case R8_R8:
    case SP_HL:
        write_register_16bit(in.reg1, read_register_16bit(in.reg2));
        break;

    case D16_REL_SP:
        write_memory_16bit(in.address, in.data);
        break;

    case HL_REL_D8: // load immediate value from operands
        write_memory(in.address, in.data);
        break;

    default:
        if (IS_DST_REGISTER(in))
            write_register_16bit(in.reg1, in.data);
        else // load value from 8bit register source
            write_memory(in.address, read_register(in.reg1));
        break;
    }

    return in.cycle_count;
}

INSTRUCTION(ldh)
{
    if (IS_DST_REGISTER(in))
        write_register(in.reg1, in.data);
    else
        write_memory(in.address, read_register_16bit(in.data));
    return in.cycle_count;
}

INSTRUCTION(di)
{
    interrupt_set_ime(false);
    return in.cycle_count;
}

INSTRUCTION(ei)
{
    cpu.ime_scheduled = true;
    return in.cycle_count;
}

INSTRUCTION(ccf)
{
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_C, !get_flag(FLAG_C));
    return in.cycle_count;
}

INSTRUCTION(scf)
{
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_C, true);
    return in.cycle_count;
}

INSTRUCTION(daa)
{
    u8 reg_a = read_register(REG_A);
    u8 flag_c = get_flag(FLAG_C);

    if (!get_flag(FLAG_N)) { // Add
        if (flag_c || reg_a > 0x99) {
            reg_a += 0x60;
            set_flag(FLAG_C, true);
        }
        if (get_flag(FLAG_H) || (reg_a & 0x0F) > 0x09) {
            reg_a += 0x06;
        }
    } else { // Sub
        if (flag_c) {
            reg_a -= 0x60;
        }
        if (get_flag(FLAG_H)) {
            reg_a -= 0x06;
        }
    }

    write_register(REG_A, reg_a);

    set_flag(FLAG_H, false);
    set_flag(FLAG_Z, reg_a == 0);

    return in.cycle_count;
}

INSTRUCTION(cpl)
{
    write_register(REG_A, ~read_register(REG_A));
    set_flag(FLAG_N, true);
    set_flag(FLAG_H, true);
    return in.cycle_count;
}

INSTRUCTION(push)
{
    stack_push_16bit(read_register_16bit(in.reg1));
    return in.cycle_count;
}

INSTRUCTION(pop)
{
    write_register_16bit(in.reg1, stack_pop_16bit());
    // Don't overwrite F's unused bits
    cpu.registers.f = cpu.registers.f & 0xF0;
    return in.cycle_count;
}

INSTRUCTION(halt)
{
    cpu.halt = true;
    return in.cycle_count;
}

INSTRUCTION(inc)
{
    u8 added = 1;
    u16 base_val = (in.type == HL_REL) ? read_memory(in.address)
                                       : read_register_16bit(in.reg1);

    if (in.type == HL_REL)
        write_memory(in.address, read_memory(in.address) + 1);
    else
        write_register_16bit(in.reg1, read_register_16bit(in.reg1) + 1);

    // INC on 16bit registers doesn't affect condition bits.
    if (in.type == R16)
        return in.cycle_count;

    set_flag(FLAG_N, false);

    if (in.type == R16) {
        set_flag(FLAG_H, ((base_val & 0xFFF) + (added & 0xFFF)) > 0xFFF);
        set_flag(FLAG_Z, (base_val + 1) == 0);
    } else {
        set_flag(FLAG_Z, (base_val & 0xFF) == 0xFF);
        set_flag(FLAG_H, ((base_val & 0xF) + (added & 0xF)) > 0xF);
    }

    return in.cycle_count;
}

// Available addressing mode: R16, R8, HL_REL
INSTRUCTION(dec)
{
    u16 base_val = (in.type == HL_REL) ? read_memory(in.address)
                                       : read_register_16bit(in.reg1);

    if (in.type == HL_REL)
        write_memory(in.address, read_memory(in.address) - 1);
    else
        write_register_16bit(in.reg1, read_register_16bit(in.reg1) - 1);

    // DEC on 16bit registers doesn't affect condition bits.
    if (in.type == R16)
        return in.cycle_count;

    set_flag(FLAG_Z, (base_val - 1) == 0);
    set_flag(FLAG_N, 1);
    if (in.type == R16) { // 16-bit subtraction
        set_flag(FLAG_H, ((base_val - 1) & 0x0FFF) == 0x0FFF);
    } else {
        set_flag(FLAG_H, ((base_val - 1) & 0x0F) == 0x0F);
    }

    return in.cycle_count;
}

#pragma region add_sub

INSTRUCTION(sub)
{
    u16 val = read_register_16bit(in.reg1);
    u16 data = (in.type == A_R8) ? read_register(in.reg2) : in.data;

    set_flag(FLAG_N, true);
    set_flag(FLAG_Z, val - data == 0);
    set_flag(FLAG_C, data > val);
    set_flag(FLAG_H, ((val & 0xF) - (data & 0xF)) < 0);

    write_register(in.reg1, val - data);

    return in.cycle_count;
}

INSTRUCTION(sbc)
{
    u8 val = read_register(in.reg1);
    u16 subbed = (in.type == A_HL_REL || in.type == A_D8)
                   ? in.data
                   : read_register_16bit(in.reg2);

    u8 h = (val & 0xF) - (subbed & 0xF) - (get_flag(FLAG_C));
    u16 c = (val) - (subbed) - (get_flag(FLAG_C));

    val -= subbed + get_flag(FLAG_C);
    write_register(in.reg1, val);

    set_flag(FLAG_N, true);
    set_flag(FLAG_Z, val == 0);
    set_flag(FLAG_H, h & 0x10);
    set_flag(FLAG_C, c & 0x100);

    return in.cycle_count;
}

INSTRUCTION(add)
{
    u16 val = read_register_16bit(in.reg1);
    u16 data = (in.type == A_R8 || in.type == HL_R16)
                 ? read_register_16bit(in.reg2)
                 : in.data;

    set_flag(FLAG_N, false);

    if (in.type == HL_R16) { // 16-bit addition
        set_flag(FLAG_C, (val + data) & 0x10000);
        set_flag(FLAG_H, ((val & 0xFFF) + (data & 0xFFF)) & 0x1000);
    } else {
        set_flag(FLAG_C, ((val & 0xFF) + (data & 0xFF)) & 0x100);
        set_flag(FLAG_H, ((val & 0xF) + (data & 0xF)) & 0x10);
        set_flag(FLAG_Z, ((val + data) & 0xFF) == 0);
    }

    // No Z for 16bit addition
    if (in.type == SP_S8) {
        set_flag(FLAG_Z, false);
        write_register_16bit(in.reg1, val + (i8)data);
    } else {
        write_register_16bit(in.reg1, val + data);
    }

    return in.cycle_count;
}

INSTRUCTION(adc)
{
    u8 c = get_flag(FLAG_C);
    u16 val = read_register_16bit(in.reg1);
    u16 added = (in.type == A_HL_REL || in.type == A_D8)
                  ? in.data
                  : read_register_16bit(in.reg2);

    set_flag(FLAG_N, false);
    set_flag(FLAG_H, ((val & 0xF) + (added & 0xF) + c) & 0x10);
    set_flag(FLAG_C, (val + added + c) & 0x100);
    set_flag(FLAG_Z, ((val + added + c) & 0xFF) == 0);

    write_register_16bit(in.reg1, val + added + c);

    return in.cycle_count;
}

#pragma endregion add_sub

INSTRUCTION(and)
{
    u8 a = read_register(REG_A);

    if (in.type == A_R8)
        in.data = read_register(in.reg2);
    write_register(REG_A, a & in.data);

    set_all_flags((a & in.data) == 0, 0, 1, 0);
    return in.cycle_count;
}

INSTRUCTION(or)
{
    u8 a = read_register(REG_A);

    if (in.type == A_R8)
        in.data = read_register(in.reg2);
    write_register(REG_A, a | in.data);

    set_all_flags((a | in.data) == 0, 0, 0, 0);
    return in.cycle_count;
}

INSTRUCTION(xor)
{
    u8 a = read_register(REG_A);

    if (in.type == A_R8)
        in.data = read_register(in.reg2);
    write_register(REG_A, a ^ in.data);

    set_all_flags((a ^ in.data) == 0, 0, 0, 0);
    return in.cycle_count;
}

INSTRUCTION(cp)
{
    u8 a = read_register(REG_A);
    if (in.type == A_R8)
        in.data = read_register(in.reg2);

    set_flag(FLAG_Z, a == in.data);
    set_flag(FLAG_H, (a & 0xF) < (in.data & 0xF));
    set_flag(FLAG_C, a < in.data);
    set_flag(FLAG_N, true);

    return in.cycle_count;
}

INSTRUCTION(rla)
{
    u8 a = read_register(REG_A);
    u8 c = get_flag(FLAG_C);

    set_flag(FLAG_C, BIT(a, 7)); // Copy 7th bit form A to carry flag

    a = (a << 1) | c;
    write_register(REG_A, a);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, false);

    return in.cycle_count;
}

INSTRUCTION(rlca)
{
    u8 a = read_register(REG_A);

    set_flag(FLAG_C, BIT(a, 7)); // Copy 7th bit from A to carry flag

    a = (a << 1) + BIT(a, 7);
    write_register(REG_A, a);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, false);

    return in.cycle_count;
}

INSTRUCTION(rra)
{
    u8 a = read_register(REG_A);
    u8 c = get_flag(FLAG_C);
    set_flag(FLAG_C, BIT(a, 0));
    write_register(REG_A, (a >> 1) + (c << 7));

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, false);

    return in.cycle_count;
}

INSTRUCTION(rrca)
{
    u8 a = read_register(REG_A);
    set_flag(FLAG_C, BIT(a, 0));
    write_register(REG_A, (a >> 1) + (get_flag(FLAG_C) << 7));

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, false);

    return in.cycle_count;
}

INSTRUCTION(stop)
{
    not_implemented(__FUNCTION__);
    return in.cycle_count;
}

#pragma region cb_instructions
#pragma endregion cb_instructions

static u8 cb(__attribute__((unused)) struct instruction in)
{
    return cb_execute_instruction();
}

// clang-format off

static in_handler instruction_handlers[] = {
    [IN_ERR] = invalid,
    [IN_NOP] = nop,
    [IN_JP] = jp,
    [IN_JR] = jr,
    [IN_CALL] = call,
    [IN_RET] = ret,
    [IN_RETI] = reti,
    [IN_RST] = rst,
    [IN_LD] = ld,
    [IN_LDH] = ldh,
    [IN_DI] = di,
    [IN_EI] = ei,
    [IN_CCF] = ccf,
    [IN_SCF] = scf,
    [IN_DAA] = daa,
    [IN_CPL] = cpl,
    [IN_PUSH] = push,
    [IN_POP] = pop,
    [IN_INC] = inc,
    [IN_DEC] = dec,
    [IN_ADD] = add,
    [IN_ADC] = adc,
    [IN_SUB] = sub,
    [IN_SBC] = sbc,
    [IN_AND] = and,
    [IN_OR] = or,
    [IN_XOR] = xor,
    [IN_CP] = cp,
    [IN_RLA] = rla,
    [IN_RLCA] = rlca,
    [IN_RRA] = rra,
    [IN_RRCA] = rrca,
    [IN_STOP] = stop,
    [IN_HALT] = halt,
    [IN_CB] = cb,
};

// clang-format on

u8 execute_instruction()
{
    u8 opcode = fetch_opcode();
    struct instruction in = fetch_instruction(opcode);

    return instruction_handlers[in.instruction](in);
}
