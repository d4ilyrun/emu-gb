#pragma once

#include "utils/types.h"

/*
 *
 * 0x0104 - 0x0133 - Nintendo logo
 *
 * This 48 bytes represent the Nintendo logo that is shown when the Game Boy is
 * powered on (the ones that appear corrupted if the cartridge is not inserted
 * correctly, for example).
 */
static u8 nintendo_logo[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
    0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

struct game_info {
    char game_title[11];
    char manufacturer_code[4];
    u8 cbg_flag;
};

/*
 * The Cartridge Header (0x0100 - 0x014F)
 *
 * The area at 0100h-014Fh of the ROM is reserved for some special information
 */
struct cartridge_header {
    u8 start_vector[4];
    u8 nintendo_logo[sizeof(nintendo_logo)];
    struct game_info game_info;
    u16 new_license_code;
    u8 sgb_flag;
    u8 type;
    u8 rom_size;
    u8 ram_size;
    u8 dst_code;
    u8 old_license_code;
    u8 rom_version;
    u8 header_checksum;
    u16 global_checksum; // Not verified in the Game Boy
};

struct cartridge {
    char filename[1024];
    u32 rom_size;
    u8 *rom;
};

typedef enum cartridge_type
{
    ROM_ONLY = 0x0,
    MBC1     = 0x03,
    MBC2     = 0x06,
    MBC3     = 0x13,
    MBC5     = 0x1E,
    MBC6     = 0x20,
    MBC7     = 0x22,
} cartridge_type;

extern struct cartridge cartridge;

#define HEADER(_cart) ((struct cartridge_header *)((_cart).rom + 0x100))

bool load_cartridge(char *path);

void cartridge_info();

u8 read_cartridge(u16 address);
u16 read_cartridge_16bit(u16 address);
