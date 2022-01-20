#include "CPU/flag.h"
#include "CPU/instruction.h"
#include "utils/macro.h"

#define CB_INSTRUCTION(_name) static u8 _name(struct cb_instruction in)

typedef u8 (*cb_handler)(struct cb_instruction);

/*
 *  Find the CB instruction according to the opcode's structure.
 *  Please refer to the link for more explanations.
 *
 *  - http://www.z80.info/decoding.htm
 */
static struct cb_instruction fetch_cb_instruction()
{
    u8 opcode = fetch_opcode();
    u8 z = OPCODE_Z(opcode);
    struct cb_instruction cb;

    if ((cb.is_address = z == 0x6))
        cb.address = read_memory(read_register_16bit(REG_HL));
    else
        cb.reg = (z == 0x7) ? REG_A : REG_B + z;

    cb.bit = OPCODE_Y(opcode);
    cb.type = OPCODE_X(opcode);

    return cb;
}

CB_INSTRUCTION(rlc)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    set_flag(FLAG_C, BIT(val, 7)); // Copy 7th bit from value to carry flag
    val = (val << 1) + get_flag(FLAG_C);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(rrc)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    set_flag(FLAG_C, BIT(val, 0));
    val = (val >> 1) + (get_flag(FLAG_C) << 7);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(rl)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    u8 c = get_flag(FLAG_C);
    set_flag(FLAG_C, BIT(val, 7)); // Copy 7th bit from value to carry flag
    val = (val << 1) + c;

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(rr)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    u8 c = get_flag(FLAG_C);
    set_flag(FLAG_C, BIT(val, 0));
    val = (val >> 1) + (c << 7);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(sla)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    set_flag(FLAG_C, BIT(val, 7)); // Copy 7th bit from value to carry flag
    val = (val << 1);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(sra)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    u8 msb = BIT(val, 7);
    val = (val >> 1) + (msb << 7);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    // C is set to the lsb of val in the original Z80 but the GB's doc sets it
    // to 0. I don't know if it is an error or an actual change but we'll follow
    // the GB's doc.
    set_flag(FLAG_C, 0);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(swap)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    val = ((val & 0xF0) >> 4) | ((val & 0x0F) << 4);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_C, false);
    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(srl)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);
    set_flag(FLAG_C, BIT(val, 0));
    val = (val >> 1);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    set_flag(FLAG_H, false);
    set_flag(FLAG_N, false);
    set_flag(FLAG_Z, val == 0);

    return 1 + in.is_address;
}

CB_INSTRUCTION(bit)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);

    set_flag(FLAG_Z, BIT(val, in.bit));
    set_flag(FLAG_N, false);
    set_flag(FLAG_H, true);

    return 1 + in.is_address;
}

CB_INSTRUCTION(res)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);

    val = val & ~(1 << in.bit);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    return 1 + in.is_address;
}

CB_INSTRUCTION(set)
{
    u8 val = (in.is_address) ? read_memory(in.address) : read_register(in.reg);

    val = val | (1 << in.bit);

    if (in.is_address)
        write_memory(in.address, val);
    else
        write_register(in.reg, val);

    return 1 + in.is_address;
}

static cb_handler rot_list[] = {rlc, rrc, rl, rr, sla, sra, swap, srl};

u8 cb_execute_instruction()
{
    struct cb_instruction in = fetch_cb_instruction();
    cb_handler handlers[] = {rot_list[in.rot_type], bit, res, set};

    return handlers[in.type](in);
}
