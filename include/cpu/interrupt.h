#pragma once

#include "utils/types.h"

#define IE_ADDRESS 0xFFFF
#define IF_ADDRESS 0xFF0F

// TODO: Comments ! What does all this refer to !?

// Interrupt vectors
typedef enum interrupt_vector
{
    IV_NONE = 0x0000,
    IV_VBLANK = 0x0040,
    IV_LCD = 0x0048,
    IV_TIMA = 0x0050,
    IV_SERIAL = 0x0058,
    IV_JOYPAD = 0x0060,
} interrupt_vector;

/**
 * \function handle_interrupts
 * \brief Execute the next requested interrupt
 *
 * Interrupts are handled by order of importance.
 * The order is:
 *
 *   VBLANK > LCD > TIMA > SERIAL > JOYPAD
 *
 * \return The number of cycles the interrupt took, 0 if no interrupt was
 * handled.
 */
u8 handle_interrupts();

/**
 * \function interrupt_set_ime
 * \brief Set the IME's value
 *
 * 0: Disable all interrupts (even if set).
 * 1: Enable interrupts
 */
void interrupt_set_ime(bool value);

/**
 * \function interrupt_get_ime
 * \brief Return the IME's current value
 */
bool interrupt_get_ime();

/**
 * \function read_timer
 * \brief read an 8bit value inside the interrupt registers.
 * \param address 16bit memory address (xFFFF or 0xFF0F)
 * \see read_memory
 */
u8 read_interrupt(u16 address);

/**
 * \function write_timer
 * \brief write an 8bit value into the interrupt registers.
 *
 * \param address 16bit memory address (xFFFF or 0xFF0F)
 * \param val 8bit value
 */
void write_interrupt(u16 address, u8 val);

/**
 * \function interrupt_request
 * \brief Request a certain interrupt to be handled
 *
 * This function only sets the interrupt's corresponding flag
 * in the  IF register, it doesn't guarantee its handling.
 * Its flag still needs to be set inside the IE register, and
 * the IME needs to be set.
 *
 * \see interrupt_vector
 */
void interrupt_request(interrupt_vector interrupt);

/**
 * \function read_interrupt
 * \brief Know wether an interrupt's flag is set inside IF
 *
 * \return \c true if set
 */
bool interrupt_is_set(interrupt_vector interrupt);
