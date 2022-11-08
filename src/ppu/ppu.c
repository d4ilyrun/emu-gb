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
#include "ppu/lcd.h"
#include "utils/error.h"
#include "utils/macro.h"
#include "utils/types.h"

static struct ppu g_ppu;

#define TILE_MAPS 0x9800

const struct ppu *ppu_get()
{
    return &g_ppu;
}

void ppu_init()
{
    // TODO: Doubled in CGB mode (two ram banks)
    g_ppu.tile_data = calloc(0x1800, sizeof(u8));

    memset(g_ppu.oam, 0, sizeof(g_ppu.oam));

    lcd_set_mode(MODE_OAM);
}

u8 read_vram(u16 address)
{
    ASSERT_MSG(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Read out of range: " HEX, address);

    if (address >= TILE_MAPS) {
        const u16 offset = address - TILE_MAPS;
        return g_ppu.tile_maps[offset / 0x400][offset % 0x400];
    } else {
        return g_ppu.tile_data[address - ROM_BANK_SWITCHABLE];
    }
}

void write_vram(u16 address, u8 value)
{
    ASSERT_MSG(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Write out of range: " HEX, address);

    if (address >= TILE_MAPS) {
        const u16 offset = address - TILE_MAPS;
        g_ppu.tile_maps[offset / 0x400][offset % 0x400] = value;
    } else {
        g_ppu.tile_data[address - ROM_BANK_SWITCHABLE] = value;
    }
}

u8 read_oam(u16 address)
{
    ASSERT_MSG(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Read out of range: " HEX, address);

    return ((u8 *)g_ppu.oam)[address - RESERVED_ECHO_RAM];
}

void write_oam(u16 address, u8 value)
{
    ASSERT_MSG(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Write out of range: " HEX, address);

    ((u8 *)g_ppu.oam)[address - RESERVED_ECHO_RAM] = value;
}
