/**
 * \file cpu/memory.h
 *
 * Memory related methods and constants.
 *
 * The original GameBoy's memory map was separatedin multiple 'areas', each
 * serving a different purpose.
 * Use the methods within this file to safely access the data in memory, perform
 * the necessary checks or update values when needed.
 *
 * \warning Do not try to access the data in memory. Always access it thorugh
 * the functions defined in this file.
 */

#pragma once

#include "utils/types.h"

/// Cartridge memory related constants
/// Each constant represents the end of an address rang
#define MEMORY_START 0x0000
#define ROM_BANK 0x4000
#define ROM_BANK_SWITCHABLE 0x8000

#define VIDEO_RAM 0xA000
#define EXTERNAL_RAM 0xC000
#define WORK_RAM 0xD000
#define WORK_RAM_SWITCHABLE 0xE000
#define RESERVED_ECHO_RAM 0xFE00
#define OAM 0xFEA0
#define RESERVED_UNUSED 0xFF0
#define IO_PORTS 0xFF80
#define CPU_HIGH_RAM 0xFFFF
#define INTERRUPT_ENABLE_FLAGS 0xFFFF

/**
 * \brief write an 8bit value into memory.
 *
 * \param address 16bit memory address
 * \param val 8bit value
 */
void write_memory(u16 address, u8 val);

/**
 * \brief write a 16bit value into memory.
 *
 * The original GB stored values using the little endian notation.
 * This functions ensures that the bill will be correctly ordered.
 *
 * \param address 16bit memory address
 * \param val 8bit value
 */
void write_memory_16bit(u16 address, u16 val);

/**
 * \brief read an 8bit value from memory.
 *
 * \param address 16bit memory address
 * \return the 8bit value at the address
 */
u8 read_memory(u16 address);

/**
 * \brief read an 16bit value from memory.
 *
 * The original GB stored values using the little endian notation.
 * This functions ensures that the bits will be correctly ordered.
 *
 * \param address 16bit memory address
 * \return the 16bit value at the address
 */
u16 read_memory_16bit(u16 address);

/**
 * \brief Restrict access the memory in the RAM area (0xA000-0xBFFF).
 *
 * The access is restricted by default but can be activated by writing to the
 * RAMG area of the MBC1, MBC2 cartridges.
 * If writing a value with the bit patten \c 0b1010 in the lower nibble,
 * access will be granted. Any other value will disable the acess to the RAM.
 *
 * When access is disabled, all writes to the external RAM are ignored and reads
 * return undefined values.
 *
 * \see write_cartridge write_cartridge_16bit
 */
extern bool ram_access;
