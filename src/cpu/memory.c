#include "cpu/memory.h"

#include "cartridge/cartridge.h"
#include "cpu/cpu.h"
#include "cpu/interrupt.h"
#include "io.h"
#include "ppu/lcd.h"
#include "ppu/ppu.h"
#include "utils/log.h"
#include "utils/macro.h"

bool g_ram_access = false;

void write_memory(u16 address, u8 val)
{
    if (address < ROM_BANK_SWITCHABLE) {
        write_cartridge(address, val);
    } else if (address < VIDEO_RAM) {
        if (lcd_get_mode() != MODE_TRANSFER)
            write_vram(address, val);
    }

    else if (IN_RANGE(address, RESERVED_ECHO_RAM, OAM)) {
        if (lcd_get_mode() < MODE_OAM)
            write_oam(address, val);
    }

    else if (IN_RANGE(address, RESERVED_UNUSED, IO_PORTS)) {
        write_io(address, val);
    }

    else if (address == INTERRUPT_ENABLE_FLAGS) {
        write_interrupt(address, val);
    }

    else {
        // log_warn("Writing to an unsupported range: " HEX16, address);
        g_cpu.memory[address] = val;
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
    } else if (address < VIDEO_RAM) {
        if (lcd_get_mode() == MODE_TRANSFER)
            return 0xFF;
        return read_vram(address);
    }

    else if (IN_RANGE(address, RESERVED_ECHO_RAM, OAM)) {
        if (lcd_get_mode() >= MODE_OAM)
            return 0xFF;
        return read_oam(address);
    }

    else if (IN_RANGE(address, RESERVED_UNUSED, IO_PORTS)) {
        return read_io(address);
    }

    if (address == INTERRUPT_ENABLE_FLAGS) {
        return read_interrupt(address);
    }

    // log_warn("Reading from an unsupported range: " HEX16, address);

    return g_cpu.memory[address];
}

u16 read_memory_16bit(u16 address)
{
    if (address < ROM_BANK_SWITCHABLE) {
        return read_cartridge_16bit(address);
    }

    return g_cpu.memory[address] + (g_cpu.memory[address + 1] << 8);
}
