#include "CPU/instruction.h"

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

static in_handler instruction_handlers[] = {
    [IN_ERR] = invalid,
    [IN_NOP] = nop
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

