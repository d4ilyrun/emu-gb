#include "CPU/cpu.h"

#include "utils/macro.h"

struct gb_cpu cpu;

void reset_cpu()
{
    cpu.registers.pc = 0x0100; // Cartridge start vector
    cpu.registers.sp = 0xFFFE;

    // Initialize registers
    write_register_16bit(REG_AF, 0x01B0);
    write_register_16bit(REG_BC, 0x0013);
    write_register_16bit(REG_DE, 0x00D8);
    write_register_16bit(REG_HL, 0x014D);

    cpu.halt          = false;
    cpu.ime_scheduled = false;
    cpu.is_running    = true;
}

void write_register(cpu_register_name reg, u8 val)
{
    if (!IS_16BIT(reg)) {
        *(REGISTERS + reg) = val;
        return;
    }

    if (reg == REG_PC) {
        cpu.registers.pc = val;
        return;
    } else if (reg == REG_SP) {
        cpu.registers.sp = val;
        return;
    }

    *((REGISTERS) + 2 * (reg % REG_AF) + 1) = val;
}

void write_register_16bit(cpu_register_name reg, u16 val)
{
    if (!IS_16BIT(reg)) {

        *(REGISTERS + reg) = LSB(val);
        return;
    }

    if (reg == REG_PC) {
        cpu.registers.pc = val;
        return;
    } else if (reg == REG_SP) {
        cpu.registers.sp = val;
        return;
    }

    *(REGISTERS + 2 * (reg % REG_AF))     = MSB(val);
    *(REGISTERS + 2 * (reg % REG_AF) + 1) = LSB(val);
}

u8 read_register(cpu_register_name reg)
{
    if (!IS_16BIT(reg))
        return *(REGISTERS + reg);

    if (reg == REG_PC)
        return LSB(cpu.registers.pc);
    if (reg == REG_SP)
        return LSB(cpu.registers.sp);
    else
        return *(REGISTERS + 2 * (reg % REG_AF) + 1); // LSB
}

u16 read_register_16bit(cpu_register_name reg)
{
    if (!IS_16BIT(reg))
        return *(REGISTERS + reg);

    if (reg == REG_PC)
        return cpu.registers.pc;
    else if (reg == REG_SP)
        return cpu.registers.sp;

    return (*(REGISTERS + 2 * (reg % REG_AF)) << 8) +
         *(REGISTERS + 2 * (reg % REG_AF) + 1);
}
