#include "CPU/instruction.h"
#include "CPU/stack.h"

#include <err.h>

#define INSTRUCTION(_name) \
    static u8 _name(struct instruction in)

typedef u8 (*in_handler)(struct instruction);

INSTRUCTION(invalid)
{
    errx(-1, "Invalid instruction.");
}

INSTRUCTION(nop)
{
    return in.cycle_count;
}

INSTRUCTION(jp)
{
    if (!in.condition)
        return in.cycle_count_false;
    write_register_16(REG_PC, in.address);
    return in.cycle_count;
}

INSTRUCTION(jr)
{
    if (!in.condition)
        return in.cycle_count_false;
    write_register_16(REG_PC, read_register_16(REG_PC) + (i8)in.data);
    return in.cycle_count_false;
}

INSTRUCTION(call)
{
    if (!in.condition)
        return in.cycle_count_false;
    stack_push_16bit(in.address);
    return in.cycle_count;
}

INSTRUCTION(ret)
{
    if (!in.condition)
        return in.cycle_count_false;
    cpu.registers.pc = stack_pop_16bit();
    return in.cycle_count;
}

INSTRUCTION(reti)
{
    cpu.registers.pc = stack_pop_16bit();
    // TODO: update IME
    return in.cycle_count;
}

INSTRUCTION(rst)
{
    stack_push_16bit(read_register_16(REG_PC));
    write_register_16(REG_PC, in.data);
    return in.cycle_count;
}

INSTRUCTION(ld)
{
    if (in.type == R8_R8)
        write_register_16(in.reg1, read_register_16(in.reg2));
    else if (IS_DST_REGISTER(in))
        write_register_16(in.reg1, in.data);
    else if (in.type == HL_REL_R8)
        write_memory(in.address, in.reg1);
    else
        write_memory(in.address, in.data);
    return in.cycle_count;
}

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
};

static inline u8 fetch_opcode()
{
    return read_memory(cpu.registers.pc++);
}

u8 execute_instruction()
{
    u8 opcode             = fetch_opcode();
    struct instruction in = fetch_instruction(opcode);

    return instruction_handlers[in.instruction](in);
}

