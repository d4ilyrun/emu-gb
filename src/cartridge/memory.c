#include "CPU/memory.h"

#include "cartridge.h"
#include "utils/macro.h"

#define RAM_GATE 0x2000
#define ROM_BANK2 0x5FFF

/**
 * \brief MODE register
 *
 * Determines how the ROM_BANK2 register value is used during access
 *
 * 1 = ROM_BANK2 affects accesses to 0x0000-0x3FFF, 0x4000-0x7FFF, 0xA000-0xBFFF
 * 0 = ROM_BANK2 affects only accesses to 0x4000-0x7FFF
 */
static bool mode = false;

u8 read_cartridge(u16 address)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        return cartridge.rom[address];
    } else if (rom->type <= MBC1) {
    }

    return cartridge.rom[address];
}

u16 read_cartridge_16bit(u16 address)
{
    return cartridge.rom[address] + (cartridge.rom[address + 1] << 8);
}

void write_cartridge(u16 address, u8 data)
{
    struct cartridge_header *rom = HEADER(cartridge);

    if (rom->type == ROM_ONLY) {
        cartridge.rom[address] = data;
    } else if (rom->type <= MBC1) {
        if (address < RAM_GATE) {
            // bits 7-4 are ignored during write
            cartridge.rom[address] = data & 0xF;
            // Update RAM access
            ram_access = cartridge.rom[address] & 0b1010;
        } else if (address < ROM_BANK) {
            // bits 7-5 are ignored during write
            cartridge.rom[address] = data & 0x1F;
            if (!cartridge.rom[address]) // Value can never be null
                cartridge.rom[address] = 1;
        } else if (address < ROM_BANK2) {
            // bits 7-2 are ignored during write
            cartridge.rom[address] = data & 0x3;
        } else {
            // bits 7-1 are ignored during write
            cartridge.rom[address] = data & 0x1;
            mode = cartridge.rom[address]; // update mode register
        }
    }

    cartridge.rom[address] = data;
}

void write_cartridge_16bit(u16 address, u16 data)
{
    cartridge.rom[address] = LSB(data);
    cartridge.rom[address + 1] = MSB(data);
}
