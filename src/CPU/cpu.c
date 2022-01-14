#include "CPU/cpu.h"

#include "utils/macro.h"

struct gb_cpu cpu;

void reset_cpu()
{
    cpu.registers.pc = 0x0100; // Cartridge start vector
    cpu.registers.pc = 0xFFFE;

    // Initialize registers
    write_register_16(REG_AF, 0x01B0);
    write_register_16(REG_BC, 0x0013);
    write_register_16(REG_DE, 0x00D8);
    write_register_16(REG_HL, 0x014D);
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

void write_register_16(cpu_register_name reg, u16 val)
{
    if (!IS_16BIT(reg)) {
        // TODO: print context on error
        fprintf(stderr,
                "[ERROR]: trying to write 16bits into an 8bit register\n");
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
        return cpu.registers.pc & 0xFF;
    else if (reg == REG_SP)
        return cpu.registers.sp & 0xFF;
    else
        return *(((u16 *)REGISTERS) + (reg % REG_AF)) & 0xFF;
}

u16 read_register_16(cpu_register_name reg)
{
    if (!IS_16BIT(reg)) {
        // TODO: print context on error
        fprintf(stderr,
                "[ERROR]: trying to read 16bits from an 8bit register\n");
        return 0;
    }

    if (reg == REG_PC)
        return cpu.registers.pc;
    else if (reg == REG_SP)
        return cpu.registers.sp;
    else
        return *(((u16 *)REGISTERS) + (reg % REG_AF));
}
