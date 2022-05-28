/**
 * \file mbc3.c
 * \brief memory access for MBC3 cartridges
 * \author Léo DUBOIN
 */

#include <assert.h>
#include <string.h>
#include <time.h>

#include "CPU/interrupt.h"
#include "CPU/memory.h"
#include "cartridge/cartridge.h"
#include "cartridge/memory.h"
#include "utils/macro.h"

/// Currently mapped  rtc register (similar to the ram_g register for MBC1
/// cartridges)
static u8 rtc_mapped_register = 0;

struct {
    /// Actual computer clock, cannot be accessed directly.
    /// Its content shall be latched into the following registers to be read.
    time_t time;

    /// These registers are accessed when performing a read/write access
    /// They contain: seconds, minutes, hours, days (u16)
    u8 writable[5];
    u8 readable[5];
} rtc;

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

    if (!BIT(rtc.time, RTC_HALT_FLAG) && now > rtc.time)
        diff = now - rtc.time;

    diff += (time_t)rtc.writable[RTC_SECONDS];
    diff += (time_t)rtc.writable[RTC_MINUTES] * 60;
    diff += (time_t)rtc.writable[RTC_HOURS] * 3600;
    diff += (time_t)rtc.writable[RTC_DAYS_LO] * 3600 * 24;
    diff +=
        BIT((time_t)rtc.writable[RTC_DAYS_HI], DAY_UPPER_BIT) * 3600 * 24 * 256;

    rtc.writable[RTC_SECONDS] = diff % 60;
    diff /= 60;
    rtc.writable[RTC_MINUTES] = diff % 60;
    diff /= 60;
    rtc.writable[RTC_HOURS] = diff % 24;
    diff /= 24;
    rtc.writable[RTC_DAYS_LO] = diff % 256;
    diff /= 256;

    // Set upper bit of the day counter, aka bit 0 of the RTC_DAYS_HI counter
    rtc.writable[RTC_DAYS_HI] =
        (rtc.writable[RTC_DAYS_HI] & 0xFE) | BIT(diff, 1);

    // Set day cary flag. Cannot be unset unless clearly asked.
    rtc.writable[RTC_DAYS_HI] |= (diff > 0) << DAY_CARRY;

    // Update the clock timer
    rtc.time = now;
}

/// Write the content of the writable register into the readable one
void latch_rtc()
{
    update_rtc();
    memcpy(rtc.readable, rtc.writable, sizeof(rtc.writable));
}

/// FIXME: I don't really understand the documentation on this one ...
void map_rtc(const u8 data)
{
    update_rtc();
    if (data >= 0x8 && data <= 0xC)
        rtc_mapped_register = data - 0X8;
}

WRITE_FUNCTION(mbc3)
{

    // READ ONLY
    if (address < RAM_GATE) {
        // bits 7-4 are ignored during write
        chip_registers.ram_g = data & 0xF;
        // Update RAM and RTC access
        ram_access = cartridge.rom[address] & 0b1010;
    } else if (address < ROM_BANK) {
        chip_registers.bank_1 = data;
        if (!chip_registers.bank_1) // Value can never be null
            chip_registers.bank_1 = 1;
    } else if (address < ROM_BANK2) {
        // 00h - 03h: maps the corresponding external RAM bank
        if (data <= 0x3)
            chip_registers.bank_2 = data & 0x3;
        // 08h - 0Ch: maps the corresponding RTC registers
        // (checked when accessing A000h-BFFFh)
        rtc_mapped_register = data;
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
    else if (address >= VIDEO_RAM && address < EXTERNAL_RAM && ram_access) {
        if (rtc_mapped_register < 0x03)
            cartridge.rom[address] = data;
        else if (rtc_mapped_register >= 0x8 && rtc_mapped_register <= 0xC) {
            update_rtc();
            rtc.writable[rtc_mapped_register - 0x8] = data;
        }
    }
}
