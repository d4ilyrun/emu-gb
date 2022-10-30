/**
 * \file mbc3.c
 * \brief memory access for MBC3 cartridges
 * \author Léo DUBOIN
 */

#include <assert.h>
#include <string.h>
#include <time.h>

#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "cpu/interrupt.h"
#include "cpu/memory.h"
#include "utils/error.h"
#include "utils/macro.h"

#ifdef UNIT_TEST
#define STATIC
#else
#define STATIC static
#endif

/// Currently mapped  rtc register (similar to the ram_g register for MBC1
/// cartridges)
STATIC u8 g_rtc_mapped_register = 0;

struct mbc3_rtc_register {
    /// Actual computer clock, cannot be accessed directly.
    /// Its content shall be latched into the following registers to be read.
    time_t time;

    /// These registers are accessed when performing a read/write access
    /// They contain: seconds, minutes, hours, days (u16)
    u8 writable[5];
    u8 readable[5];
} g_rtc;

/// Indexes for the values in the RTC register
///
/// For more information:
/// - https://gbdev.io/pandocs/MBC3.html#the-clock-counter-registers

#define RTC_SECONDS 0
#define RTC_MINUTES 1
#define RTC_HOURS 2
#define RTC_DAYS_LO 3 // Low byte for the day counter (0-255)

// - Upper 1 bit of the day counter (bit 0)
// - Halt flag (bit 6, 1=set)
// - Day counter carry flag (bit 7)
#define RTC_DAYS_HI 4

#define DAY_UPPER_BIT 0
#define DAY_CARRY 7
#define RTC_HALT_FLAG 6

/*
 * Compute the difference between the current time and the last computed time.
 * Add it to the writable register and update the clock's time.
 *
 * If the rtc is stopped (HALT_FLAG), directly do not compute the difference and
 * directly add the current time to the counters.
 */
static void update_rtc()
{
    time_t now = time(NULL);
    time_t diff = 0;

    if (!BIT(g_rtc.time, RTC_HALT_FLAG) && now > g_rtc.time)
        diff = now - g_rtc.time;

    diff += (time_t)g_rtc.writable[RTC_SECONDS];
    diff += (time_t)g_rtc.writable[RTC_MINUTES] * 60;
    diff += (time_t)g_rtc.writable[RTC_HOURS] * 3600;
    diff += (time_t)g_rtc.writable[RTC_DAYS_LO] * 3600 * 24;
    diff += BIT((time_t)g_rtc.writable[RTC_DAYS_HI], DAY_UPPER_BIT) * 3600 *
            24 * 256;

    g_rtc.writable[RTC_SECONDS] = diff % 60;
    diff /= 60;
    g_rtc.writable[RTC_MINUTES] = diff % 60;
    diff /= 60;
    g_rtc.writable[RTC_HOURS] = diff % 24;
    diff /= 24;
    g_rtc.writable[RTC_DAYS_LO] = diff % 256;
    diff /= 256;

    // Set upper bit of the day counter, aka bit 0 of the RTC_DAYS_HI counter
    g_rtc.writable[RTC_DAYS_HI] =
        (g_rtc.writable[RTC_DAYS_HI] & 0xFE) | BIT(diff, 1);

    // Set day cary flag. Cannot be unset unless clearly asked.
    g_rtc.writable[RTC_DAYS_HI] |= (diff > 0) << DAY_CARRY;

    // Update the clock timer
    g_rtc.time = now;
}

/// Write the content of the writable register into the readable one
void latch_rtc()
{
    update_rtc();
    memcpy(g_rtc.readable, g_rtc.writable, sizeof(g_rtc.writable));
}

void map_rtc(const u8 data)
{
    update_rtc();
    if (data >= 0x8 && data <= 0xC)
        g_rtc_mapped_register = data - 0X8;
}

WRITE_FUNCTION(mbc3)
{

    // READ ONLY
    if (address < RAM_GATE) {
        // bits 7-4 are ignored during write
        g_chip_registers.ram_g = data & 0xF;
        // Update RAM and RTC access
        g_ram_access = g_cartridge.rom[address] & 0b1010;
    } else if (address < ROM_BANK) {
        g_chip_registers.rom_bank = data;
        if (!g_chip_registers.rom_bank) // Value can never be null
            g_chip_registers.rom_bank = 1;
    } else if (address < ROM_BANK2) {
        // 00h - 03h: maps the corresponding external RAM bank
        if (data <= 0x3)
            g_chip_registers.ram_bank = data & 0x3;
        // 08h - 0Ch: maps the corresponding RTC registers
        // (checked when accessing A000h-BFFFh)
        g_rtc_mapped_register = data;
    } else if (address < ROM_BANK_SWITCHABLE) {
        // When writing 00h and then 01h into this register the current time
        // becomes latched into the RTC registers.
        static u8 last_value = 0x01;
        if (data == 0x01 && last_value == 0x00)
            latch_rtc();
        last_value = data;
    }

    // MEMORY
    // Write only if access has been granted.
    // If an rtc register has been previously mapped into memory (cf ROM_BANK2
    // if clause), the rtc register value will be updated.
    // Else update the address in memory.
    else if (address >= VIDEO_RAM && address < EXTERNAL_RAM && g_ram_access) {
        if (g_rtc_mapped_register < 0x03)
            g_cartridge.rom[address] = data;
        else if (g_rtc_mapped_register >= 0x8 && g_rtc_mapped_register <= 0xC) {
            update_rtc();
            g_rtc.writable[g_rtc_mapped_register - 0x8] = data;
        }
    }
}

/**
 * \see compute_physical_addresss in mbc1.c
 */
static unsigned compute_physical_addresss(u16 address)
{
    u8 bank_size = 5;
    u8 bank = 0b0000000;

    /* Trying to write into RAM.
     *
     * In this case the physical address is a combination of the 13 lower
     * bits of the requested address and BANK2, if mode is set, 0b00 else.
     */
    if (VIDEO_RAM <= address && address < EXTERNAL_RAM) {
        bank = g_chip_registers.mode ? g_chip_registers.ram_bank : 0b00;
        return (address & 0x1FFF) + (bank << 13);
    }

    if (address < 0x4000) {
        if (g_chip_registers.mode)
            bank = g_chip_registers.ram_bank << bank_size;
    } else if (address < 0x8000) {
        bank = g_chip_registers.ram_bank << bank_size;
        bank += g_chip_registers.rom_bank;
    } else {
        ASSERT_NOT_REACHED();
    }

    return (address & 0x3FFF) + (bank << 14);
}

READ_FUNCTION(mbc3)
{
    if (address >= VIDEO_RAM && address < EXTERNAL_RAM && !g_ram_access)
        return 0xFF; // Undefined value

    if (g_rtc_mapped_register >= 0x8 && g_rtc_mapped_register <= 0xC)
        return g_rtc.readable[g_rtc_mapped_register - 0x8];

    unsigned physical_address = compute_physical_addresss(address);

    ASSERT_MSG(physical_address < g_cartridge.rom_size,
               "MBC3: Reading out of bounds (" HEX16 ")", physical_address);

    return g_cartridge.rom[physical_address];
}

DUMP_FUNCTION(mbc3)
{
    u8 num_banks = 2 << (HEADER(g_cartridge)->rom_size + 1);
    u16 bank_start;

    // This value is set but never used in the original algorithm.
    // I don't know what purpose it serves but i'll leave it anyway.
    __attribute__((unused)) unsigned buf = 0;

    write_mbc1(0x6000, 0x01);
    for (u8 bank = 0; bank < num_banks; ++bank) {
        write_mbc1(0x2000, bank);
        write_mbc1(0x4000, bank >> 5);
        bank_start = (bank & 0x1F) ? 0x4000 : 0x0000;
        for (u16 addr = bank_start; addr < bank_start + 0x4000; ++addr)
            buf += read_mbc1(addr);
    }
}
