#include "CPU/memory.h"

#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/macro.h"

struct chip_registers_t chip_registers = {0, 0, 0, false, 1};

u8 read_cartridge(u16 address)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        return cartridge.rom[address];
    } else if (rom->type <= MBC1) {
        return read_mbc1(address);
    }

    // TODO: invalid cartridge type
    return cartridge.rom[address];
}

u16 read_cartridge_16bit(u16 address)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        return cartridge.rom[address] + (cartridge.rom[address + 1] << 8);
    } else if (rom->type <= MBC1) {
        return read_mbc1_16bit(address);
    }

    // TODO: invalid cartridge type
    return cartridge.rom[address] + (cartridge.rom[address + 1] << 8);
}

void write_cartridge(u16 address, u8 data)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        cartridge.rom[address] = data;
    } else if (rom->type <= MBC1) {
        write_mbc1(address, data);
    }
}

void write_cartridge_16bit(u16 address, u16 data)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        cartridge.rom[address] = LSB(data);
        cartridge.rom[address + 1] = MSB(data);
    } else if (rom->type <= MBC1) {
        write_mbc1_16bit(address, data);
    }
}
