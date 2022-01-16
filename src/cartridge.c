#include "cartridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/macro.h"

struct cartridge cartridge;

static bool verify_header_checksum(struct cartridge cart)
{
    unsigned char sum = 0;
    int i             = 0x0134;

    while (i <= 0x014C)
        sum -= cart.rom[i++] - 1;

    return sum == 0;
}

bool load_cartridge(char *path)
{
    FILE *rom = fopen(path, "r");

    if (rom == NULL) {
        perror("Failed to load cartridge");
        return false;
    }

    strcpy(cartridge.filename, path);

    // Get rom_size and allocate enough space to store the cartridge's rom
    fseek(rom, 0, SEEK_END);
    cartridge.rom_size = ftell(rom);
    cartridge.rom      = malloc(cartridge.rom_size);

    rewind(rom);
    fread(cartridge.rom, 1, cartridge.rom_size, rom);

    if (verify_header_checksum(cartridge)) {
        fputs("Failed to load cartridge: Invalid checksum.", stderr);
        return false;
    }

    return true;
}

static void print_nintendo_logo()
{
    for (int y = 0; y < 8; ++y) {
        int i = ((y / 2) % 2) + (y / 4) * 24;
        for (int x = 0; x < 12; ++x, i += 2) {
            const uint8_t n =
                (y % 2) ? (nintendo_logo[i] & 0xF) : (nintendo_logo[i] >> 4);
            for (int b = 4; b--; )
                putchar(((n >> b) & 1) ? '*' : ' ');
        }
        putchar('\n');
    }
}

void cartridge_info()
{
    struct cartridge_header *header = HEADER(cartridge);

    print_nintendo_logo();

    puts("\nCartridge information:");
    printf("\tPath      : %s\n", cartridge.filename);
    printf("\tTitle     : %s\n", header->game_info.game_title);
    printf("\tROM Size  : %d KB\n", 32 << header->rom_size);
    printf("\tRAM Size  : %2.2X\n", header->ram_size);
    printf("\tROM Vers  : %2.2X\n", header->rom_version);
}

// TODO: follow the cartridge's specifications
u8 read_cartridge(u16 address)
{
    return cartridge.rom[address];
}

u16 read_cartridge_16bit(u16 address)
{
    return cartridge.rom[address] + (cartridge.rom[address + 1] << 8);
}
