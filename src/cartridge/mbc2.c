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

#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "cpu/cpu.h"
#include "cpu/memory.h"
#include "utils/error.h"
#include "utils/macro.h"

static unsigned compute_physical_adress(u16 address);

WRITE_FUNCTION(mbc2)
{
    // READ ONLY
    if (address < ROM_BANK && !BIT(address, 8)) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        ram_access = chip_registers.ram_g == 0xA;
    } else if (address < ROM_BANK && BIT(address, 8)) {
        // bits 7-4 are ignored during write
        chip_registers.rom_bank = data & 0xF;
        if (!chip_registers.rom_bank) // Can never be null
            chip_registers.rom_bank = 0b0001;
    }

    // READ/WRITE AREA
    else if (VIDEO_RAM <= address && address < EXTERNAL_RAM && ram_access) {
        // Upper 4 bits are ignored
        const u16 physical_address = compute_physical_adress(address);
        assert_msg(physical_address < cartridge.ram_size,
                   "MBC2: writing outside of RAM bounds");
        cartridge.ram[physical_address] = data & 0xF;
    }
}

/**
 * Calculate the actual physical address within the ROM from the value inside
 * the ROMB register.
 *
 * Physical address is:
 * - 0x0000 - 0x3FFF: 0000 + 14 lower bits of address
 * - 0x4000 - 0x7FFF: ROMB + 14 lower bits of address
 */
static unsigned compute_physical_adress(u16 address)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM)
        return address - 0xA000;

    if (address < ROM_BANK)
        return address & 0x1FFF;
    if (address < ROM_BANK_SWITCHABLE) {
        // Switch to rom bank 1 of set to 0
        if (!chip_registers.rom_bank)
            return (1 << 14) | (address & 0x3FFF);
        return (chip_registers.rom_bank << 14) + (address & 0x3FFF);
    }

    assert_not_reached();
    // fatal_error("MBC2: compute_physical_adress: invalid area");
}

READ_FUNCTION(mbc2)
{
    const bool is_ram = VIDEO_RAM <= address && address < EXTERNAL_RAM;

    if (is_ram && !ram_access)
        return 0xFF; // Undefined value

    unsigned physical_address = compute_physical_adress(address);

    if (is_ram) {
        assert_msg(physical_address < cartridge.ram_size,
                   "MBC2: Reading outside of RAM bounds");
        return cartridge.ram[physical_address];
    }

    assert_msg(physical_address < cartridge.rom_size,
               "MBC2: Reading outside of ROM bounds");
    return cartridge.rom[physical_address];
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
