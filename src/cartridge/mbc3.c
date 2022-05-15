/**
 * \file mbc3.c
 * \brief memory access for MBC3 cartridges
 * \author Léo DUBOIN
 */

#include <assert.h>

#include "CPU/memory.h"
#include "cartridge/cartridge.h"
#include "cartridge/memory.h"

static bool rtc_access = false;
static u8 rtc = 0;

WRITE_FUNCTION(mbc3)
{

    // READ ONLY
    if (address < RAM_GATE) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        // Update RAM and RTC access
        ram_access = cartridge.rom[address] & 0b1010;
        rtc_access = cartridge.rom[address] & 0b1010;
    } else if (address < ROM_BANK) {
        chip_registers.bank_1 = data;
        if (!chip_registers.bank_1) // Value can never be null
            chip_registers.bank_1 = 1;
    } else if (address < ROM_BANK2) {
        // 00h - 03h: maps the corresponding external RAM bank
        chip_registers.bank_2 = data & 0x3;
        // 08h - 0Ch: maps the corresponding RTC register
        rtc = (data >> 2) & 0x3;
    }
}
