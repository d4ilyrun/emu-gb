#include "CPU/memory.h"

#include "CPU/cpu.h"
#include "cartridge.h"
#include "utils/macro.h"

// TODO: handle different cases in memory map

// TODO: Actually write in memory lol
void write_memory(u16 address, u8 val)
{
    if (address < ROM_BANK_SWITCHABLE) {
        write_cartridge(address, val);
        return;
    }

    cpu.memory[address] = val;
}

void write_memory_16bit(u16 address, u16 val)
{
    if (address < ROM_BANK_SWITCHABLE) {
        write_cartridge_16bit(address, val);
        return;
    }

    // Inverse byte order
    cpu.memory[address] = LSB(val);
    cpu.memory[address + 1] = MSB(val);
}

u8 read_memory(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE) {
        return read_cartridge(address);
    }

    return cpu.memory[address];
}

u16 read_memory_16bit(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE) {
        return read_cartridge_16bit(address);
    }

    return cpu.memory[address] + (cpu.memory[address + 1] << 8);
}
