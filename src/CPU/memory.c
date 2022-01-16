#include "CPU/memory.h"

#include "CPU/cpu.h"
#include "utils/macro.h"
#include "cartridge.h"

// TODO: handle different cases in memory map

void write_memory(u16 address, u8 val)
{
    if (address < ROM_BANK_SWITCHABLE)
    {

    }

    NOT_IMPLEMENTED(__FUNCTION__ );
}


void write_memory_16bit(u16 address, u16 val)
{
    if (address < ROM_BANK_SWITCHABLE)
    {

    }

    NOT_IMPLEMENTED(__FUNCTION__ );
}

u8 read_memory(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE)
    {
        return read_cartridge(address);
    }

    return cpu.memory[address];
}

u16 read_memory_16bit(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE)
    {
        return read_cartridge_16bit(address);
    }

    return cpu.memory[address] + (cpu.memory[address + 1] << 8);
}
