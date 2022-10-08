/**
 * \file fifo.c
 * \author Léo DUBOIN <leo@duboin.com>
 *
 * <h1>FIFO Pixel Fetcher</h1>
 *
 * The fetcher fetches a row of 8 background or window pixels and queues them up
 * to be mixed with sprite pixels.
 *
 * The pixel fetcher has 5 steps.
 * The first four steps take 2 dots each and the fifth step is attempted every
 * dot until it succeeds. The order of the steps are as follows:
 *
 *  1. Get tile
 *  2. Get tile data low
 *  3. Get tile data high
 *  4. Sleep
 *  5. Push
 */

#include "ppu/fifo.h"

#include <stdlib.h>
#include <string.h>

#include "ppu/lcd.h"
#include "ppu/ppu.h"
#include "utils/error.h"
#include "utils/types.h"

/*
 * There are two pixel FIFOs. One for background pixels and one for OAM (sprite)
 * pixels. They are independent of each other.
 * The two FIFOs are mixed only when popping items.
 */

/*
static struct pixel_fetcher fifo_bg = {0};
static struct pixel_fetcher fifo_oam = {0};
*/

#pragma region fifo

static void fifo_push(struct fifo *fifo, pixel pixel)
{
    if (fifo->count >= FIFO_MAX_LENGTH) {
        log_warn("FIFO: Already full. Skipping.");
        return;
    }

    struct fifo_node *new = malloc(sizeof(struct fifo));

    new->pixel = pixel;
    new->next = NULL;

    if (fifo->count > 0)
        fifo->tail->next = new;
    else
        fifo->head = new;

    fifo->tail = new;
    fifo->count += 1;
}

__attribute__((unused)) static pixel fifo_pop(struct fifo *fifo)
{
    if (fifo->count == 0) {
        log_warn("FIFO: Popping from empty fifo. Skipping.");
        return (pixel)0xFFFFFFFF;
    }

    struct fifo_node *head = fifo->head;
    pixel pixel = head->pixel;

    fifo->head = head->next;
    fifo->count -= 1;

    free(head);

    return pixel;
}

#pragma endregion fifo

#pragma region fetcher

void fetcher_reset(struct pixel_fetcher *fetcher)
{
    // Empty FIFO
    struct fifo_node *node = fetcher->fifo.head;
    while (node != fetcher->fifo.tail) {
        struct fifo_node *tmp = node;
        node = node->next;
        free(tmp);
    }

    free(node);

    // Reset values
    const fifo_type type = fetcher->fifo.type;
    memset(fetcher, 0, sizeof(struct pixel_fetcher));
    fetcher->fifo.type = type;

    // Set some values
    fetcher->scanline_y = get_lcd()->ly;
    fetcher->tile_y = fetcher->scanline_y % 8;
}

/**
 * \function fifo_get_tile
 * \brief Determine which background/window tile to fetch pixels from
 *
 * \param fetcher The pixel fetcher used
 * \return The tile index of the itle to use
 *
 * \param fetcher The fetcher used
 *
 * \see https://gbdev.io/pandocs/pixel_fifo.html#get-tile
 */
static inline u8 fetcher_get_tile(struct pixel_fetcher *fetcher)
{
    u16 tile_map;
    const struct lcd *lcd = get_lcd();
    u8 fetcher_x = fetcher->scanline_x + lcd->scx;
    u8 fetcher_y = fetcher->scanline_y + lcd->scy;

    bool window_tile = fetcher->scanline_x > lcd->wx
                    && fetcher->scanline_y > lcd->wy;

    // if scanline is in window: select window tile_map
    // else: select BG tile_map
    tile_map = window_tile ? LCD_WINDOW_TILE_MAP(*lcd) : LCD_BG_TILE_MAP(*lcd);

    // Tiles are 8*8 pixels, so this value changes every 8 pixels
    // (Each scanline for X, and every 8 lines for Y)
    // We multiply Y by 32 because there are 32 tiles per line (32*32)
    return read_vram(tile_map + (fetcher_x / 8) + (fetcher_y / 8) * 32);
}

/**
 * \function fetcher_get_data
 * \brief Store the tile_data for the current tile inside the pixel array
 *
 * Corresponds to steps 2 and 3 of the cycle.
 *
 * \param fetcher   The fetcher used
 * \param high      Wether to return low or high tile data (true = step 3)
 *
 * \see https://gbdev.io/pandocs/pixel_fifo.html#get-tile-data-low
 * \see https://gbdev.io/pandocs/pixel_fifo.html#get-tile-data-high
 */
static inline void fetcher_get_data(struct pixel_fetcher *fetcher, bool high)
{
    const u8 data = read_vram(TILE_DATA + (fetcher->tile_id * 16)
                              + (fetcher->tile_y * 2) + high);

    // Pixels are stored in reverse order here (easier to write ^^')
    for (u8 pixel = 0; pixel < PIXELS_PER_SCANLINE; ++pixel) {
        if (!high) {
            fetcher->pixels[pixel].pixel.color = BIT(data, pixel);
        } else {
            fetcher->pixels[pixel].pixel.color |= BIT(data, pixel) << 1;
        }
    }
}

/**
 * \function fetcher_push_row
 * \brief Push a row of pixels to the FIFO
 *
 * Corresponds to step 5 of the cycle.
 *
 * \param fetcher   The fetcher used
 *
 * \see https://gbdev.io/pandocs/pixel_fifo.html#push
 */
static void fetcher_push_row(struct pixel_fetcher *fetcher)
{
    if (fetcher->fifo.count + PIXELS_PER_SCANLINE > FIFO_MAX_LENGTH) {
        log_err("Pixel Fetcher: not enough space in FIFO during PUSH step.");
        return;
    }

    for (i8 i = PIXELS_PER_SCANLINE - 1; i >= 0; --i) {
        pixel px = fetcher->pixels[i];

        // Black pixel if bg/window not enabled
        if (fetcher->fifo.type == FIFO_BG) {
            if (LCD_BG_WINDOW_ENABLED(*get_lcd()))
                px.pixel.color = lcd_get_palette(BG_PALETTE)[px.pixel.color];
            else
                px.pixel.color = lcd_get_palette(BG_PALETTE)[0];
        } else {
            not_implemented("FIFO OAM");
            return;
        }

        fifo_push(&fetcher->fifo, px);
    }
}

void fetcher_process(struct pixel_fetcher *fetcher)
{
    // Run at half the PPU's speed: every 2 clock cycles
    if (fetcher->tick++ % 2)
        return;

    switch (fetcher->step) {
    case PF_FETCH:
        fetcher->tile_id = fetcher_get_tile(fetcher);
        // Update to next tile
        fetcher->scanline_x += 8; // 8 pixels per tile
        fetcher->step = PF_DATA_0;
        break;

    case PF_DATA_0:
        fetcher_get_data(fetcher, false);
        fetcher->step = PF_DATA_1;
        break;

    case PF_DATA_1:
        fetcher_get_data(fetcher, true);
        fetcher->step = PF_IDLE;
        break;

    case PF_IDLE:
        fetcher->step = PF_PUSH;
        break;

    case PF_PUSH:
        fetcher_push_row(fetcher);
        fetcher->step = PF_FETCH;
        break;

    default:
        log_err("Unknown pixel fetcher step: %d", fetcher->step);
        break;
    }
}

#pragma endregion fetcher
