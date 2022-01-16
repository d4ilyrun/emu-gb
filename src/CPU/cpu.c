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
        *((u8 *)REGISTERS + reg) = val;
        return;
    }

    if (reg == REG_PC)
        cpu.registers.pc = val;
    else if (reg == REG_SP)
        cpu.registers.sp = val;
    else
        *(((u16 *)REGISTERS) + (reg % REG_AF)) = val;
}

void write_register_16bit(cpu_register_name reg, u16 val)
{
    if (!IS_16BIT(reg)) {
        *((u8 *)REGISTERS + reg) = LSB(val);
    }

    if (reg == REG_PC)
        cpu.registers.pc = val;
    else if (reg == REG_SP)
        cpu.registers.sp = val;
    else
        *(((u16 *)REGISTERS) + (reg % REG_AF)) = val;
}

u8 read_register(cpu_register_name reg)
{
    if (!IS_16BIT(reg))
        return *((u8 *)REGISTERS + reg);

    if (reg == REG_PC)
        return LSB(cpu.registers.pc);
    else if (reg == REG_SP)
        return LSB(cpu.registers.sp);
    else
        return LSB(*(((u16 *)REGISTERS) + (reg % REG_AF)));
}

u16 read_register_16bit(cpu_register_name reg)
{
    if (!IS_16BIT(reg))
        return *((u8 *)REGISTERS + reg);

    if (reg == REG_PC)
        return cpu.registers.pc;
    else if (reg == REG_SP)
        return cpu.registers.sp;
    else
        return *(((u16 *)REGISTERS) + (reg % REG_AF));
}
