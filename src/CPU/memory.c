#include "CPU/memory.h"

#include "CPU/cpu.h"
#include "CPU/interrupt.h"
#include "cartridge/cartridge.h"
#include "io.h"
#include "utils/log.h"
#include "utils/macro.h"

bool ram_access = false;

void write_memory(u16 address, u8 val)
{
    if (address < ROM_BANK_SWITCHABLE) {
        write_cartridge(address, val);
    }

    else if (IN_RANGE(address, RESERVED_UNUSED, IO_PORTS)) {
        write_io(address, val);
    }

    else if (address == INTERRUPT_ENABLE_FLAGS) {
        write_interrupt(address, val);
    }

    else {
        // log_warn("Writing to an unsupported range: " HEX16, address);
        cpu.memory[address] = val;
    }
}

void write_memory_16bit(u16 address, u16 val)
{
    write_memory(address, LSB(val));
    write_memory(address + 1, MSB(val));
}

u8 read_memory(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE) {
        return read_cartridge(address);
    }

    else if (IN_RANGE(address, RESERVED_UNUSED, IO_PORTS)) {
        return read_io(address);
    }

    else if (address == INTERRUPT_ENABLE_FLAGS) {
        return read_interrupt(address);
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
