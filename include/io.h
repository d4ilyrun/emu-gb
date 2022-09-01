#pragma once

#include "utils/types.h"

/**
 * \function write_timer
 * \brief write an 8bit value into the IO registers.
 *
 * \param address 16bit memory address between 0xFF00 - 0xFF7F
 * \param val 8bit value
 */
void write_io(u16 address, u8 data);

/**
 * \function read_timer
 * \brief read a 8bit value from the IO registers.
 * \param address 16bit memory address between 0xFF00 - 0xFF7F
 * \see read_memory
 */
u8 read_io(u16 address);
