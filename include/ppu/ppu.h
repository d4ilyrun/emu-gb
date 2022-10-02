#pragma once

#include "utils/types.h"

/**
 * \union oam_data
 * \brief typed of the data contained inside the OAM memory
 *
 * \see https://gbdev.io/pandocs/OAM.html
 */
typedef union {

    /// Fields with their actual meaning
    struct oam_data {

        u8 pos_y; ///< Sprite’s vertical position on the screen + 16
        u8 pos_x; ///< Sprite’s horizontal position on the screen + 8
        u8 tile_index;

        /**
         * __Bit7__   BG and Window over OBJ
         *       __ (0=No, 1=BG and Window colors 1-3 over the OBJ) <br>
         * __Bit6__   Y flip          (0=Normal, 1=Vertically mirrored) <br>
         * __Bit5__   X flip          (0=Normal, 1=Horizontally mirrored) <br>
         * __Bit4__   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
         * <br>
         * __Bit3__   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank
         * 1)<br>
         * __Bit2-0__ Palette number  **CGB Mode Only**     (OBP0-7) <br>
         */
        u8 flags;

    } oam;

    /// Raw representation (4 bytes)
    u32 raw;

} oam_data;

struct ppu {
    /**
     * \brief The ppu's vram area for Tile data.
     *
     * Tile data is stored in VRAM in the memory area at $8000-$97FF; with each
     * tile taking 16 bytes, this area defines data for 384 tiles.
     * In CGB Mode, this is doubled (768 tiles) because of the two VRAM banks.
     */
    u8 *tile_data;

    /**
     * \brief VRAM Tile Maps
     *
     * The Game Boy contains two 32x32 tile maps in VRAM at the memory areas
     * $9800-$9BFF and $9C00-$9FFF. Any of these maps can be used to display the
     * Background or the Window.
     */
    u8 tile_maps[2][0x400];

    /**
     * \brief VRAM Sprite Attribute Table
     *
     * Sprite attributes reside in the Sprite Attribute Table (OAM - Object
     * Attribute Memory) at $FE00-FE9F. Each of the 40 entries consists of four
     * bytes with a specific meaning.
     */
    oam_data oam[40];
};

/**
 * \function ppu_get
 * \brief Get the emulator's actual ppu instance
 * \return A pointer to the static ppu struct used by the emulator.
 */
const struct ppu *ppu_get();

/**
 * \function ppu_init
 * \brief Init the picture processing unit (PPU) and all its components
 */
void ppu_init();

/**
 * \function read_vram
 * \brief read a 8bit value from the ppu's vram.
 *
 * \param address 16bit memory address between 0x8000 - 0x97FF
 * \see read_memory
 */
u8 read_vram(u16 address);

/**
 * \function write_vram
 * \brief write an 8bit value into the ppu's vram.
 *
 * \param address 16bit memory address between 0x8000 - 0x97FF
 * \param val 8bit value
 * \see write_memory
 */
void write_vram(u16 address, u8 value);

/**
 * \function read_oam
 * \brief read a 8bit value from the ppu's sprite attribute table.
 *
 * \param address 16bit memory address between 0xFE00 - 0xFE9F
 * \see read_memory
 */
u8 read_oam(u16 address);

/**
 * \function write_oam
 * \brief write an 8bit value into the ppu's sprite attribute table.
 *
 * \param address 16bit memory address between 0xFE00 - 0xFE9F
 * \param val 8bit value
 * \see write_memory
 */
void write_oam(u16 address, u8 value);
