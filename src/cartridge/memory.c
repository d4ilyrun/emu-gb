#include "cpu/memory.h"

#include <assert.h>

#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/log.h"
#include "utils/macro.h"

struct chip_registers_t g_chip_registers = {0, 1, 0, false};

u8 read_cartridge(u16 address)
{
    struct cartridge_header *rom_ptr = HEADER(g_cartridge);

    if (rom_ptr->type == ROM_ONLY) {
        assert(address < g_cartridge.rom_size);
        return g_cartridge.rom[address];
    }
    if (rom_ptr->type <= MBC1) {
        return read_mbc1(address);
    }
    if (rom_ptr->type <= MBC2) {
        return read_mbc2(address);
    }
    if (rom_ptr->type <= MBC3) {
        return read_mbc3(address);
    }

    log_err("Unsupported cartdrige type: " HEX, rom_ptr->type);

    return g_cartridge.rom[address];
}

u16 read_cartridge_16bit(u16 address)
{
    return read_cartridge(address) + (read_cartridge(address + 1) << 8);
}

void write_cartridge(u16 address, u8 data)
{
    struct cartridge_header *rom_ptr = HEADER(g_cartridge);

    if (rom_ptr->type == ROM_ONLY) {
        assert(address < g_cartridge.rom_size);
        g_cartridge.rom[address] = data;
    } else if (rom_ptr->type <= MBC1) {
        write_mbc1(address, data);
    } else if (rom_ptr->type <= MBC2) {
        write_mbc2(address, data);
    } else if (rom_ptr->type <= MBC3) {
        write_mbc3(address, data);
    } else {
        log_err("Unsupported cartdrige type: " HEX, rom_ptr->type);
    }
}

void write_cartridge_16bit(u16 address, u16 data)
{
    write_cartridge(address, LSB(data));
    write_cartridge(address + 1, MSB(data));
}
