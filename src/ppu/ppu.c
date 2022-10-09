/**
 * \file ppu.c
 * \author Léo DUBOIN <leo@duboin.com>
 *
 * The PPU (picture processing unit) is responsible for storing adn rendering
 * pixels.
 *
 * \link https://gbdev.io/pandocs/Rendering.html
 */

#include "ppu/ppu.h"

#include <string.h>

#include "cpu/cpu.h"
#include "cpu/memory.h"
#include "ppu/fifo.h"
#include "ppu/lcd.h"
#include "utils/error.h"
#include "utils/macro.h"
#include "utils/types.h"

static struct ppu ppu;

#define TILE_MAPS 0x9800

const struct ppu *ppu_get()
{
    return &ppu;
}

void ppu_init()
{
    // TODO: Doubled in CGB mode (two ram banks)
    ppu.tile_data = calloc(0x1800, sizeof(u8));

    memset(ppu.oam, 0, sizeof(ppu.oam));

    lcd_set_mode(MODE_OAM);
    ppu.ticks = 0;
}

u8 read_vram(u16 address)
{
    assert_msg(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Read out of range: " HEX, address);

    if (address >= TILE_MAPS) {
        const u16 offset = address - TILE_MAPS;
        return ppu.tile_maps[offset / 0x400][offset % 0x400];
    } else {
        return ppu.tile_data[address - ROM_BANK_SWITCHABLE];
    }
}

void write_vram(u16 address, u8 value)
{
    assert_msg(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Write out of range: " HEX, address);

    if (address >= TILE_MAPS) {
        const u16 offset = address - TILE_MAPS;
        ppu.tile_maps[offset / 0x400][offset % 0x400] = value;
    } else {
        ppu.tile_data[address - ROM_BANK_SWITCHABLE] = value;
    }
}

u8 read_oam(u16 address)
{
    assert_msg(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Read out of range: " HEX, address);

    return ((u8 *)ppu.oam)[address - RESERVED_ECHO_RAM];
}

void write_oam(u16 address, u8 value)
{
    assert_msg(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Write out of range: " HEX, address);

    ((u8 *)ppu.oam)[address - RESERVED_ECHO_RAM] = value;
}

void ppu_tick()
{
    const struct lcd *lcd = get_lcd();

    switch (LCD_STAT_MODE_FLAG(*lcd)) {
    case MODE_OAM:
        // TODO: OAM SEARCH
        // OAM SEARCH always takes 20 ticks
        if (ppu.ticks == 40) {
            fetcher_reset(&fifo_bg);
            lcd_set_mode(MODE_TRANSFER);
        }
        break;

    case MODE_TRANSFER:
        fetcher_tick(&fifo_bg);
        // Pixel fifo has to contain at least 8 pixels
        if (fifo_bg.fifo.count > 8)
            fetcher_pop_pixel(&fifo_bg);
        if (fifo_bg.scanline_x == 160) // End of line
            lcd_set_mode(MODE_HBLANK);
        break;

    case MODE_HBLANK:
        // A full scanline takes 456 ticks to complete
        if (ppu.ticks == 456) {
            ppu.ticks = 0;      // Reset for next scanline
            lcd_increment_ly(); // Next line
            // Switch to vblank once done with all visible scanlines
            if (lcd->ly == 144) // reached last line
                lcd_set_mode(MODE_VBLANK);
            else // Restart with next scanline
                lcd_set_mode(MODE_OAM);
        }
        break;

    case MODE_VBLANK:
        // A full scanline takes 456 ticks to complete
        if (ppu.ticks == 456) {
            ppu.ticks = 0;      // Reset for next scanline
            lcd_increment_ly(); // Next line
            if (lcd->ly == 0)   // looped back to first line
                lcd_set_mode(MODE_OAM);
        }
        break;

    default:
        log_err("PPU: Unsupported state: %d", LCD_STAT_MODE_FLAG(*lcd));
        break;
    }

    ppu.ticks += 1;
}
