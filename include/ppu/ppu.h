#pragma once

#include "utils/types.h"

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
    u8 tile_maps[2][0x4000];

    /**
     * \brief VRAM Sprite Attribute Table
     *
     * Sprite attributes reside in the Sprite Attribute Table (OAM - Object
     * Attribute Memory) at $FE00-FE9F. Each of the 40 entries consists of four
     * bytes with a specific meaning.
     */
    u8 oam[0x80];
};

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
