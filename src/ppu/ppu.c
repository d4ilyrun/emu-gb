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
#include "utils/error.h"
#include "utils/macro.h"
#include "utils/types.h"

static struct ppu ppu;

#define TILE_DATA 0x97FF

const struct ppu *ppu_get()
{
    return &ppu;
}

void ppu_init()
{
    // TODO: Doubled in CGB mode (two ram banks)
    ppu.tile_data = calloc(0x1800, sizeof(u8));

    memset(ppu.oam, 0, sizeof(ppu.oam));
}

u8 read_vram(u16 address)
{
    assert_msg(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Read out of range: " HEX, address);

    return ppu.tile_data[address - ROM_BANK_SWITCHABLE];
}

void write_vram(u16 address, u8 value)
{
    assert_msg(IN_RANGE(address, ROM_BANK_SWITCHABLE, VIDEO_RAM),
               "VRAM: Write out of range: " HEX, address);

    ppu.tile_data[address - ROM_BANK_SWITCHABLE] = value;
}

u8 read_oam(u16 address)
{
    assert_msg(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Read out of range: " HEX, address);

    // TODO: this only works during the HBlank and VBlank periods.
    return ppu.oam[address - RESERVED_ECHO_RAM];
}

void write_oam(u16 address, u8 value)
{
    assert_msg(IN_RANGE(address, RESERVED_ECHO_RAM, OAM),
               "OAM: Write out of range: " HEX, address);

    // TODO: this only works during the HBlank and VBlank periods.
    ppu.oam[address - RESERVED_ECHO_RAM] = value;
}
