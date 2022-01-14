#pragma once

#include "utils/types.h"

typedef enum {
    // 8 bit registers
    REG_A = 0x0,
    REG_F,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_H,
    REG_L,

    // 16 bit registers
    REG_PC,
    REG_SP,

    // combined 8 bit registers
    REG_AF,
    REG_BC,
    REG_DE,
    REG_HL,

    // invalid
    REG_ERR,
} cpu_register_name;

extern char *register_names[];

#define IS_16BIT(_reg) ((_reg) >= REG_PC)
#define IS_PAIRED(_reg) ((_reg) >= REG_AF)

struct cpu_registers{
    // 8 bit registers
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    // 16 bit registers
    u16 pc;
    u16 sp;
};

struct gb_cpu {
    // cpu registers
    struct cpu_registers registers;

    // cpu memory
    u8 memory[1 << 16];
};

// The actual CPU of the GameBoy
extern struct gb_cpu cpu;

#define REGISTERS (&(cpu.registers))

void reset_cpu();

u8 read_register(cpu_register_name register);
u16 read_register_16(cpu_register_name register);

void write_register(cpu_register_name register, u8 val);
void write_register_16(cpu_register_name register, u16 val);
