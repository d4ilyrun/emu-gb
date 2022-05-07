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
 *
 * \Todo: handle the different cases in memory map
 */

#pragma once

#include "utils/types.h"

/// Cartridge memory related constants
#define MEMORY_START 0x0000
#define ROM_BANK 0x4000
#define ROM_BANK_SWITCHABLE 0x8000

#define VIDEO_RAM 0xA000
#define EXTERNAL_RAM 0xC000
#define WORK_RAM 0xD000
#define WORK_RAM_SWITCHABLE 0xE000
#define RESERVED_ECHO_RAM 0xFE00
#define OAM 0xFEA0
#define RESERVED_UNUSED 0xFF00
#define IO_PORTS 0xFF80
#define CPU_HIGH_RAM 0xFFFF
#define INTERRUPT_ENABLE_FLAGS 0xFFFF

/**
 * \Brief write an 8bit value into memory.
 *
 * \param address 16bit memory address
 * \param val 8bit value
 */
void write_memory(u16 address, u8 val);

/**
 * \Brief write a 16bit value into memory.
 *
 * The original GB stored values using the little endian notation.
 * This functions ensures that the bill will be correctly ordered.
 *
 * \param address 16bit memory address
 * \param val 8bit value
 */
void write_memory_16bit(u16 address, u16 val);

/**
 * \Brief read an 8bit value from memory.
 *
 * \param address 16bit memory address
 * \return the 8bit value at the address
 */
u8 read_memory(u16 address);

/**
 * \Brief read an 16bit value from memory.
 *
 * The original GB stored values using the little endian notation.
 * This functions ensures that the bits will be correctly ordered.
 *
 * \param address 16bit memory address
 * \return the 16bit value at the address
 */
u16 read_memory_16bit(u16 address);
