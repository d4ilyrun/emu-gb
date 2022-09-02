/**
 * \file memory.h
 * \brief memory access for the different cartridge types
 *
 * This file contains declaration for low level functions.
 * It should not be included as memory access must be done through the higher
 * level associated functions.
 *
 * \see write_memory write_cartridge read_memory read_cartridge
 */

#pragma once

#include "utils/types.h"

/**
 * \struct chip_registers_t
 * \brief Chipset registers
 *
 * Registers are integrated within the chipset and influence how we access
 * memory. Their value are updated when writing to specific areas within hte
 * cartridge ROM memory.
 *
 * The registers vary between chipset models as does the way they impact memory
 * access.
 */
extern struct chip_registers_t {
    // MBC1 registers

    u8 ram_g;

    /**
     * Determines which rom bank to access.
     *
     * Set to 1 by default.
     */
    u8 rom_bank;

    /**
     * Determines which rom bank to access.
     * Only used by MBC1 cartridges.
     *
     * Set to 1 by default.
     */
    u8 ram_bank;

    /**
     * Determines how the ROM_BANK2 register value is used during access
     *
     * 1 = ROM_BANK2 affects accesses to 0x0000-0x3FFF, 0x4000-0x7FFF,
     * 0xA000-0xBFFF \n
     * 0= ROM_BANK2 only affects accesses to 0x4000-0x7FFF
     */
    bool mode;
} chip_registers;

// Cartridge register addresses
#define RAM_GATE 0x2000
#define ROM_BANK 0x4000 ///< \see cpu/memory
#define ROM_BANK2 0x6000
#define ROM_BANK_SWITCHABLE 0x8000 ///< \see cpu/memory.

// MEMORY ACCESS API DECLARATION

#define WRITE_FUNCTION(_type) void write_##_type(u16 address, u8 data)
#define WRITE_16_FUNCTION(_type) \
    void write_##_type##_16bit(u16 address, u16 data)

#define READ_FUNCTION(_type) u8 read_##_type(u16 address)
#define READ_16_FUNCTION(_type) u16 read_##_type##_16bit(u16 address)

#define DUMP_FUNCTION(_type) void dump_##_type()

/// Declare the necessary functions to access memory for a given cartridge type
#define DECLARE_CARTRIDGE_TYPE(_type) \
    WRITE_FUNCTION(_type);            \
    WRITE_16_FUNCTION(_type);         \
    READ_FUNCTION(_type);             \
    READ_16_FUNCTION(_type);          \
    DUMP_FUNCTION(_type);

// Currently available cartridge types are declared here:
DECLARE_CARTRIDGE_TYPE(mbc1)
DECLARE_CARTRIDGE_TYPE(mbc2)
DECLARE_CARTRIDGE_TYPE(mbc3)
