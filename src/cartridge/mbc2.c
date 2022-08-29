/**
 * \file mbc2.c
 * \brief memory API for MBC2 cartridges
 * \author Léo DUBOIN
 *
 * (max 256 KiB ROM and 512x4 bits RAM)
 *
 * The MBC2 doesn’t support external RAM, instead it includes 512x4 bits of
 * built-in RAM (in the MBC2 chip itself). It still requires an external battery
 * to save data during power-off though. As the data consists of 4bit values,
 * only the lower 4 bits of the bit octets in the RAM area (A000-A1FF) are used.
 * The upper 4 bits of each byte are undefined and should not be relied upon.
 */

#include <assert.h>

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/macro.h"

WRITE_FUNCTION(mbc2)
{
    // READ ONLY
    if (address < ROM_BANK && !BIT(address, 8)) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        // Update RAM access
        ram_access = cartridge.rom[address] & 0b1010;
    } else if (address < ROM_BANK && BIT(address, 8)) {
        // bits 7-4 are ignored during write
        chip_registers.rom_b = data & 0xF;
        if (!chip_registers.rom_b) // Can never be null
            chip_registers.rom_b = 0b0001;
    }

    // READ/WRITE AREA
    else if (VIDEO_RAM <= address && address < EXTERNAL_RAM && ram_access) {
        // Upper 4 bits are ignored
        cpu.memory[address] = data & 0xF;
    }
}

WRITE_16_FUNCTION(mbc2)
{
    // TODO: check for actual implementation

    // Write the lower bytes after so that we keep the correct lower bits for
    // when we update the registers
    write_mbc2(address + 1, MSB(data));
    write_mbc2(address, LSB(data));
}

/**
 * Calculate the actual physical address within the ROM from the value inside
 * the ROMB register.
 *
 * Physical address is:
 * - 0x0000 - 0x3FFF: 0000 + 13 lower bits of address
 * - 0x4000 - 0x7FFF: ROMB + 13 lower bits of address
 */
static unsigned compute_physical_adress(u16 address)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM)
        return address;

    if (address < ROM_BANK)
        return address & 0x3FF;
    if (address < ROM_BANK_SWITCHABLE)
        return (chip_registers.rom_b << 13) + (address & 0x3FF);

    assert(false && "MBC2: compute_physical_adress: invalid area");

    return 0xFF; // TODO: never reached macro
}

READ_FUNCTION(mbc2)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM && !ram_access)
        return 0xFF; // Undefined value

    unsigned physical_address = compute_physical_adress(address);

    // TODO: gracefully throw an error
    assert(physical_address < cartridge.rom_size);

    return cartridge.rom[physical_address];
}

READ_16_FUNCTION(mbc2)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM && !ram_access)
        return 0xFFFF; // Undefined value

    unsigned physical_address = compute_physical_adress(address);

    // TODO: gracefully throw an error
    assert(physical_address < cartridge.rom_size);

    return cartridge.rom[physical_address] + (cartridge.rom[address + 1] << 8);
}

DUMP_FUNCTION(mbc2)
{
    u8 num_banks = 2 << (HEADER(cartridge)->rom_size + 1);
    unsigned buf = 0;

    for (u8 bank = 0; bank < num_banks; ++bank) {
        write_mbc1(0x2100, bank);
        u16 bank_start = bank ? 0x4000 : 0x0000;
        for (u16 addr = bank_start; addr < bank_start + 0x4000; ++addr)
            buf += read_mbc1(addr);
    }
}
