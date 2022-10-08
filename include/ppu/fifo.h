/**
 * \file fifo.h
 * \author léo duboin <leo@duboin.com>
 *
 * <h1>fifo pixel fetcher</h1>
 *
 * the fetcher fetches a row of 8 background or window pixels and queues them up
 * to be mixed with sprite pixels.
 *
 * the pixel fetcher has 5 steps.
 * the first four steps take 2 dots each and the fifth step is attempted every
 * dot until it succeeds. the order of the steps are as follows:
 *
 *  1. get tile
 *  2. get tile data low
 *  3. get tile data high
 *  4. sleep
 *  5. push
 */

#pragma once

#include "utils/types.h"

#define PIXELS_PER_SCANLINE 8
#define FIFO_MAX_LENGTH     16

/**
 * \struct pixel
 * \brief A single pixel inside the PPUs' pixel FIFOs
 *
 * Each pixel in the FIFO has four properties:
 *  - Color:                A value between 0 and 3
 *  - Palette:              On CGB a value between 0 and 7 and on DMG this only
 *                          applies to sprites
 *  - Sprite Priority:      On CGB this is the OAM index for the sprite and on
 *                          DMG this doesn’t exist
 *  - Background Priority:  Holds the value of the OBJ-to-BG Priority bit
 *
 *  \see https://gbdev.io/pandocs/pixel_fifo.html#introduction
 */
typedef union {
    struct {
        u8 color;
        u8 palette;
        u8 sp_priority; ///< Sprite priority
        u8 bg_priority; ///< Background priority
    } pixel;
    u32 raw;
} pixel;

/**
 * \enum fifo_type
 *
 * There are two pixel FIFOs. One for background pixels and one for OAM (sprite)
 * pixels.
 */
typedef enum { FIFO_BG, FIFO_OAM } fifo_type;

/**
 * \struct fifo_node
 * \brief A queue node for the PPU FIFO struct.
 * \see ppu_fifo
 */
struct fifo_node {
    pixel pixel;
    struct fifo_node *next;
};

typedef enum {
    PF_FETCH = 0,
    PF_DATA_0,
    PF_DATA_1,
    PF_IDLE,
    PF_PUSH,
    PF_ERROR
} pixel_fetcher_step;

/**
 * \struct ppu_fifo
 * \brief A pixel FIFO
 *
 * The FIFO and Pixel Fetcher work together to ensure that the FIFO always
 * contains at least 8 pixels at any given time, as 8 pixels are required for
 * the Pixel Rendering operation to take place. Each FIFO is manipulated only
 * during mode 3 (pixel transfer).
 */
struct pixel_fetcher {
    /* FIFO */
    struct fifo {
        fifo_type type;         ///< The type of the FIFO
        struct fifo_node *head; ///< The first pixel in order (first pushed)
        struct fifo_node *tail; ///< The last pixel in order (last pushed)
        u8 count; ///< The number of pixels currently inside the FIFO
    } fifo;

    /* FETCHER */

    // The fetcher's current tick count
    // The fetcher runs a half the PPU's speed, so each step is executed during
    // odd tick cycles only.
    u16 tick;

    // The 8 pixels fetched during the current scanline
    // They are stored in reverse order (last pixel first) !
    pixel pixels[PIXELS_PER_SCANLINE];

    pixel_fetcher_step step; ///< The current step

    u8 scanline_x; ///< The X coordinate of the current scanline
    u8 scanline_y; ///< The X coordinate of the current scanline

    // Which line inside a tile the current scanline correponds to.
    // A tile is 8x8 pixels wide, so this number can be between 0 & 7
    u8 tile_y;

    u8 tile_id; ///< The tile id of the current scan line (fetched during step 1)
};

/**
 * \function fetcher_reset
 * \brief Reset a pixel fetcher to its default state
 *
 * All registers are cleared and the previous content of its FIFO
 * queue is free'd (if any).
 *
 * \param fetcher The pixel fetcher to reset
 */
void fetcher_reset(struct pixel_fetcher *fetcher);

/**
 * \function fetcher_tick
 * \brief Emulate a single tick for the pixel fetcher
 *
 * The pixel fetcher runs at half the speed of the PPU. Thus, 1 out of 2 calls to
 * this function will simply be ignored.
 *
 * \param fetcher The fetcher for which to simulate a tick
 */
void fetcher_tick(struct pixel_fetcher *fetcher);
