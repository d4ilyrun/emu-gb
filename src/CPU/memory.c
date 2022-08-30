#include "CPU/memory.h"

#include "CPU/cpu.h"
#include "cartridge/cartridge.h"
#include "utils/log.h"
#include "utils/macro.h"

bool ram_access = false;

void write_memory(u16 address, u8 val)
{
    if (address < ROM_BANK_SWITCHABLE) {
        write_cartridge(address, val);
        return;
    }

    // log_warn("Writing to an unsupported range: " HEX16, address);
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

    // log_warn("Reading from an unsupported range: " HEX16, address);

    return cpu.memory[address];
}

u16 read_memory_16bit(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE) {
        return read_cartridge_16bit(address);
    }

    return cpu.memory[address] + (cpu.memory[address + 1] << 8);
}
