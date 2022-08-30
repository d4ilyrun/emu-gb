#include "cartridge/cartridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CPU/memory.h"
#include "cartridge/memory.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/macro.h"

struct cartridge cartridge;

static bool verify_header_checksum(struct cartridge cart)
{
    unsigned char sum = 0;
    int i = 0x0134;

    while (i <= 0x014C)
        sum -= cart.rom[i++] - 1;

    return sum == 0;
}

/* Checking if an MBC1 cartridge contains multiple games.
 *
 * This is done by checking if multiple nintendo logos are present within the
 * rom.
 */
static bool check_multicart()
{
    int nb_games = 4;

    // ALl known multicart cartridges use 8M of ROM
    if (HEADER(cartridge)->rom_size < 8) {
        cartridge.multicart = false;
        return false;
    }

    // First assume that it is a multicart
    cartridge.multicart = true;

    // Set BANK1 to zero
    u8 bank2 = chip_registers.ram_bank;
    chip_registers.rom_bank = 0;
    chip_registers.mode = true;

    // Loop through all four possiblbbe BANK2 values
    for (chip_registers.ram_bank = 0b00; chip_registers.ram_bank <= 0b11;
         ++chip_registers.ram_bank) {
        // Look for the nintendo logo
        log_info("NEW BANK: %d\n", chip_registers.ram_bank);
        for (size_t i = 0; i < sizeof(nintendo_logo); ++i) {
            printf("%x = %x, ", read_cartridge(0x0104 + i), nintendo_logo[i]);
            if (read_cartridge(0x0104 + i) != nintendo_logo[i]) {
                nb_games -= 1;
                break;
            }
        }
        printf("\n");
    }

    log_info("%d\n", nb_games);
    cartridge.multicart = nb_games;
    chip_registers.ram_bank = bank2;
    chip_registers.mode = false; // Always initialized at false
    return cartridge.multicart;
}

bool load_cartridge(char *path)
{
    FILE *rom = fopen(path, "r");

    if (rom == NULL) {
        fatal_error("Failed to load cartridge: Invalid file (%s)", path);
    }

    // TODO: check if filename is too long !!!
    strcpy(cartridge.filename, path);

    // Get rom_size and allocate enough space to store the cartridge's rom
    fseek(rom, 0, SEEK_END);
    cartridge.rom_size = ftell(rom);
    cartridge.rom = malloc(cartridge.rom_size);
    cartridge.multicart = false;

    const struct cartridge_header *header = HEADER(cartridge);

    // Do the same for the RAM
    // RAM size equivalent to the code inside the header:
    //
    // $00 = 0          No RAM
    // $01 = Unused
    // $02 = 8 KiB      1 bank
    // $03 = 32 KiB     4 banks of 8 KiB each
    // $04 = 128 KiB    16 banks of 8 KiB each
    // $05 = 64 KiB     8 banks of 8 KiB each
    const u8 ram_size_code = header->ram_size;
    switch (ram_size_code) {
    case 2:
        cartridge.ram_size = 2 << 13;
        break;
    case 3:
        cartridge.ram_size = 2 << 15;
        break;
    case 4:
        cartridge.ram_size = 2 << 17;
        break;
    case 5:
        cartridge.ram_size = 2 << 16;
        break;
    default:
        cartridge.ram_size = 0;
        break;
    }

    // If is MBC2: 512*4 bit internal RAM, no external RAM
    if (header->rom_version > MBC1 && header->rom_version <= MBC2) {
        cartridge.ram_size = 512;
    }

    cartridge.ram = malloc(cartridge.ram_size ? cartridge.ram_size : 1);

    rewind(rom);
    fread(cartridge.rom, 1, cartridge.rom_size, rom);

    if (verify_header_checksum(cartridge)) {
        fatal_error("Failed to load cartridge: Invalid checksum");
    }

    cartridge_type type = HEADER(cartridge)->type;
    if (type != ROM_ONLY && type <= MBC1) // If of type MBC1
        check_multicart();

    return true;
}

static void print_nintendo_logo()
{
    for (int y = 0; y < 8; ++y) {
        int i = ((y / 2) % 2) + (y / 4) * 24;
        for (int x = 0; x < 12; ++x, i += 2) {
            const uint8_t n =
                (y % 2) ? (nintendo_logo[i] & 0xF) : (nintendo_logo[i] >> 4);
            for (int b = 4; b--;)
                putchar(((n >> b) & 1) ? '*' : ' ');
        }
        putchar('\n');
    }
}

static const char *ROM_TYPES[] = {
    "ROM ONLY",
    "MBC1",
    "MBC1+RAM",
    "MBC1+RAM+BATTERY",
    "0x04 ???",
    "MBC2",
    "MBC2+BATTERY",
    "0x07 ???",
    "ROM+RAM 1",
    "ROM+RAM+BATTERY 1",
    "0x0A ???",
    "MMM01",
    "MMM01+RAM",
    "MMM01+RAM+BATTERY",
    "0x0E ???",
    "MBC3+TIMER+BATTERY",
    "MBC3+TIMER+RAM+BATTERY 2",
    "MBC3",
    "MBC3+RAM 2",
    "MBC3+RAM+BATTERY 2",
    "0x14 ???",
    "0x15 ???",
    "0x16 ???",
    "0x17 ???",
    "0x18 ???",
    "MBC5",
    "MBC5+RAM",
    "MBC5+RAM+BATTERY",
    "MBC5+RUMBLE",
    "MBC5+RUMBLE+RAM",
    "MBC5+RUMBLE+RAM+BATTERY",
    "0x1F ???",
    "MBC6",
    "0x21 ???",
    "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
};

void cartridge_info()
{
    struct cartridge_header *header = HEADER(cartridge);

    print_nintendo_logo();

    log_info("Cartridge information:");
    log_info("\tPath      : %s", cartridge.filename);
    log_info("\tTitle     : %s", header->game_info.game_title);
    log_info("\tType      : %s", ROM_TYPES[header->rom_version]);
    log_info("\tROM Size  : %X KB", cartridge.rom_size);
    log_info("\tRAM Size  : %2.2X KB", cartridge.ram_size);
    log_info("\tMulticart : %s", cartridge.multicart ? "YES" : "NO");
}
