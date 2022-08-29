/**
 * \file mbc1.c
 * \brief memory access for MBC1 cartridges
 * \author Léo DUBOIN
 *
 * (max 2MByte ROM and/or 32 KiB RAM)
 *
 * In its default configuration, MBC1 supports up to 512 KiB ROM
 * with up to 32 KiB of banked RAM.
 * Some cartridges wire the MBC differently, where the 2-bit RAM banking
 * register is wired as an extension of the ROM banking register (instead of to
 * RAM) in order to support up to 2 MiB ROM, at the cost of only supporting a
 * fixed 8 KiB of cartridge RAM.
 */

#include <assert.h>

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/macro.h"

WRITE_FUNCTION(mbc1)
{
    // READ_ONLY AREA
    // Only update registers values when writing
    if (address < RAM_GATE) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        // FIXME: Euh ... just ... why? makes no sense ... what was i thinking?
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

    // READ/WRITE AREA
    else if (VIDEO_RAM <= address && address < EXTERNAL_RAM && ram_access) {
        cpu.memory[address] = data;
    }
}

WRITE_16_FUNCTION(mbc1)
{
    // TODO: check for actual implementation

    // Write the lower bytes after so that we keep the correct lower bits for
    // when we update the registers
    write_mbc1(address + 1, MSB(data));
    write_mbc1(address, LSB(data));
}

/**
 * Calculate the actual physical address within the ROM from teh value inside
 * the MODE and BANK registers.
 *
 * The pysical address is a combination of 7 bits formed from the value within
 * the registers (BANK) and the 14 lower bits of the requested address.
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
 * For more explanations please refer to the given documentation.
 */
static unsigned compute_physical_addresss(u16 address)
{
    // In case the chip is MBC1M, the BANK1 register is actually 4 bit long
    u8 bank_size = cartridge.multicart ? 4 : 5;
    u8 bank = 0b0000000;

    /* Trying to write into RAM.
     *
     * In this case the physical address is a combination of the 13 lower
     * bits of the requested address and BANK2, if mode is set, 0b00 else.
     */
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM) {
        bank = chip_registers.mode ? chip_registers.bank_2 : 0b00;
        return (address & 0x1FFF) + (bank << 13);
    }

    if (address < 0x4000) {
        if (chip_registers.mode)
            bank = chip_registers.bank_2 << bank_size;
    } else if (address < 0x8000) {
        bank = chip_registers.bank_2 << bank_size;
        bank += cartridge.multicart
                  ? chip_registers.bank_1 & 0xF // Only 4 first bits
                  : chip_registers.bank_1;
    } else {
        // TODO: error trying to access invalid address
        assert(false && "MBC1: Reading an out of bounds address.");
    }

    // printf("computed: %X\n", (address & 0x3FFF) + (bank << 14));

    return (address & 0x3FFF) + (bank << 14);
}

READ_FUNCTION(mbc1)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM && !ram_access)
        return 0xFF; // Undefined value

    unsigned physical_address = compute_physical_addresss(address);

    // TODO: gracefully throw an error
    assert(physical_address < cartridge.rom_size);

    return cartridge.rom[physical_address];
}

READ_16_FUNCTION(mbc1)
{
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM && !ram_access)
        return 0xFFFF; // Undefined value

    unsigned physical_address = compute_physical_addresss(address);

    // TODO: gracefully throw an error
    assert(physical_address < cartridge.rom_size);

    return cartridge.rom[physical_address] + (cartridge.rom[address + 1] << 8);
}

DUMP_FUNCTION(mbc1)
{
    u8 num_banks = 2 << (HEADER(cartridge)->rom_size + 1);
    u16 bank_start;

    // This value is set but never used in the original algorithm.
    // I don't know what purpose it serves but i'll leave it anyway.
    unsigned buf = 0;

    write_mbc1(0x6000, 0x01);
    for (u8 bank = 0; bank < num_banks; ++bank) {
        write_mbc1(0x2000, bank);
        if (cartridge.multicart) {
            write_mbc1(0x4000, bank >> 4);
            bank_start = (bank & 0x0F) ? 0x4000 : 0x0000;
        } else {
            write_mbc1(0x4000, bank >> 5);
            bank_start = (bank & 0x1F) ? 0x4000 : 0x0000;
        }
        for (u16 addr = bank_start; addr < bank_start + 0x4000; ++addr)
            buf += read_mbc1(addr);
    }
}
