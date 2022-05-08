/**
 * \file mbc1.c
 * \brief memory access for MBC1 cartridges
 * \author Léo DUBOIN
 */

#include <assert.h>

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "cartridge/cartridge.h"
#include "cartridge/memory.h"

WRITE_FUNCTION(mbc1)
{
    if (address < RAM_GATE) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        // Update RAM access
        ram_access = cartridge.rom[address] & 0b1010;
    } else if (address < ROM_BANK) {
        // bits 7-5 are ignored during write
        chip_registers.bank_1 = data & 0x1F;
        if (!chip_registers.bank_1) // Value can never be null
            chip_registers.bank_1 = 1;
    } else if (address < ROM_BANK2) {
        // bits 7-2 are ignored during write
        chip_registers.bank_2 = data & 0x3;
    } else if (address < ROM_BANK_SWITCHABLE) {
        // bits 7-1 are ignored during write
        chip_registers.mode = data & 0x1;
    }

    // FIXME: write inside ROM or heap ?
    cartridge.rom[address] = data;
}

/**
 * Calculate the actual physical address within the ROM from teh value inside
 * the MODE and BANK registers.
 *
 * The pysical address is a combination of 7 bits formed from the value within
 * the registers (BANK) and the 13 lower bits of the requested address.
 *
 * IF address in 0x0000 - 0x3FFF:
 *  IF MODE = 0 -> BANK = 0
 *  ELSE        -> BANK = combination of BANK2 and 5 null bits
 * ELSE:
 *  BANK = combination of both BANK registers
 *
 *  Physical address representation:
 *  ______________________
 *  | 20 - 14 |  13 - 0  |
 *  ----------------------
 *  |  BANK   |  A<13:0> |
 *  ----------------------
 *
 */
READ_FUNCTION(mbc1)
{
    u8 bank = 0b0000000;

    if (address < 0x4000 && chip_registers.mode) {
        bank = chip_registers.bank_2 << 5;
    } else if (address < 0x8000) {
        bank = (chip_registers.bank_2 << 5) + chip_registers.bank_1;
    } else {
        // TODO: error trying to access invalid address
        assert(false && "MBC1: Reading an out of bounds address.");
    }

    unsigned int physical_address = (address & 0x1FFF) + (bank << 13);

    // TODO: gracefully throw an error
    assert(physical_address < cartridge.rom_size);

    return cartridge.rom[physical_address];
}
