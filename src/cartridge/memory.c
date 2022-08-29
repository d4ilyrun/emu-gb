#include "CPU/memory.h"

#include <assert.h>

#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/log.h"
#include "utils/macro.h"

struct chip_registers_t chip_registers = {0, 1, 0, false};

u8 read_cartridge(u16 address)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        assert(address < cartridge.rom_size);
        return cartridge.rom[address];
    } else if (rom->type <= MBC1) {
        return read_mbc1(address);
    } else if (rom->type <= MBC2) {
        return read_mbc2(address);
    } else if (rom->type <= MBC3) {
        return read_mbc3(address);
    }

    log_err("Unsupported cartdrige type: " HEX, rom->type);

    return cartridge.rom[address];
}

u16 read_cartridge_16bit(u16 address)
{
    return read_cartridge(address) + (read_cartridge(address + 1) << 8);
}

void write_cartridge(u16 address, u8 data)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        assert(address < cartridge.rom_size);
        cartridge.rom[address] = data;
    } else if (rom->type <= MBC1) {
        write_mbc1(address, data);
    } else if (rom->type <= MBC2) {
        write_mbc2(address, data);
    } else if (rom->type <= MBC3) {
        write_mbc3(address, data);
    }

    log_err("Unsupported cartdrige type: " HEX, rom->type);
}

void write_cartridge_16bit(u16 address, u16 data)
{
    write_cartridge(address, LSB(data));
    write_cartridge(address + 1, MSB(data));
}
