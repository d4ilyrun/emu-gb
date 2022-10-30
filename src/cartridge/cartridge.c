#include "cartridge/cartridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartridge/memory.h"
#include "cpu/memory.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/macro.h"

struct cartridge g_cartridge;

static bool verify_header_checksum(struct cartridge cart)
{
    unsigned char sum = 0;

    for (u16 i = 0; i <= 0x014C; ++i)
        sum -= cart.rom[i] - 1;

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
    if (HEADER(g_cartridge)->rom_size < 8) {
        g_cartridge.multicart = false;
        return false;
    }

    // First assume that it is a multicart
    g_cartridge.multicart = true;

    // Set BANK1 to zero
    u8 bank2 = g_chip_registers.ram_bank;
    g_chip_registers.rom_bank = 0;
    g_chip_registers.mode = true;

    // Loop through all four possiblbbe BANK2 values
    for (g_chip_registers.ram_bank = 0b00; g_chip_registers.ram_bank <= 0b11;
         ++g_chip_registers.ram_bank) {
        // Look for the nintendo logo
        log_info("NEW BANK: %d\n", g_chip_registers.ram_bank);
        for (size_t i = 0; i < sizeof(g_nintendo_logo); ++i) {
            printf("%x = %x, ", read_cartridge(0x0104 + i), g_nintendo_logo[i]);
            if (read_cartridge(0x0104 + i) != g_nintendo_logo[i]) {
                nb_games -= 1;
                break;
            }
        }
        printf("\n");
    }

    log_info("%d\n", nb_games);
    g_cartridge.multicart = nb_games;
    g_chip_registers.ram_bank = bank2;
    g_chip_registers.mode = false; // Always initialized at false
    return g_cartridge.multicart;
}

bool load_cartridge(char *path)
{
    FILE *rom_ptr = fopen(path, "r");

    if (rom_ptr == NULL) {
        FATAL_ERROR("Failed to load cartridge: Invalid file (%s)", path);
    }

    // TODO: check if filename is too long !!!
    strncpy(g_cartridge.filename, path, ROM_MAX_FILENAME_SIZE - 1);

    // Get rom_size and allocate enough space to store the cartridge's rom
    if (fseek(rom_ptr, 0, SEEK_END) == -1)
        FATAL_ERROR("fseek failed");
    g_cartridge.rom_size = ftell(rom_ptr);
    g_cartridge.rom = malloc(g_cartridge.rom_size);
    g_cartridge.multicart = false;

    // Read the rom file's content into its streuct represenation
    rewind(rom_ptr);
    if (fread(g_cartridge.rom, 1, g_cartridge.rom_size, rom_ptr) == 0)
        FATAL_ERROR("fread failed");

    const struct cartridge_header *header_ptr = HEADER(g_cartridge);

    // Do the same for the RAM
    // RAM size equivalent to the code inside the header:
    //
    // $00 = 0          No RAM
    // $01 = Unused
    // $02 = 8 KiB      1 bank
    // $03 = 32 KiB     4 banks of 8 KiB each
    // $04 = 128 KiB    16 banks of 8 KiB each
    // $05 = 64 KiB     8 banks of 8 KiB each
    const u8 ram_size_code = header_ptr->ram_size;
    switch (ram_size_code) {
    case 2:
        g_cartridge.ram_size = 2 << 13;
        break;
    case 3:
        g_cartridge.ram_size = 2 << 15;
        break;
    case 4:
        g_cartridge.ram_size = 2 << 17;
        break;
    case 5:
        g_cartridge.ram_size = 2 << 16;
        break;
    default:
        g_cartridge.ram_size = 0;
        break;
    }

    // If is MBC2: 512*4 bit internal RAM, no external RAM
    if (header_ptr->rom_version > MBC1 && header_ptr->rom_version <= MBC2) {
        g_cartridge.ram_size = 512;
    }

    g_cartridge.ram = malloc(g_cartridge.ram_size ? g_cartridge.ram_size : 1);

    if (verify_header_checksum(g_cartridge)) {
        FATAL_ERROR("Failed to load cartridge: Invalid checksum");
    }

    cartridge_type type = HEADER(g_cartridge)->type;
    if (type != ROM_ONLY && type <= MBC1) // If of type MBC1
        check_multicart();

    return true;
}

static void print_nintendo_logo()
{
    for (int y = 0; y < 8; ++y) {
        int i = ((y / 2) % 2) + (y / 4) * 24;
        for (int x = 0; x < 12; ++x, i += 2) {
            const uint8_t n = (y % 2) ? (g_nintendo_logo[i] & 0xF)
                                      : (g_nintendo_logo[i] >> 4);
            for (int b = 4; b--;)
                putchar(((n >> b) & 1) ? '*' : ' ');
        }
        putchar('\n');
    }
}

static const char *g_rom_types[] = {
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
    struct cartridge_header *header_ptr = HEADER(g_cartridge);

    print_nintendo_logo();

    log_info("Cartridge information:");
    log_info("\tPath      : %s", g_cartridge.filename);
    log_info("\tTitle     : %s", header_ptr->game_info.game_title);
    log_info("\tType      : %s", g_rom_types[header_ptr->rom_version]);
    log_info("\tROM Size  : %X KB", g_cartridge.rom_size);
    log_info("\tRAM Size  : %2.2X KB", g_cartridge.ram_size);
    log_info("\tMulticart : %s", g_cartridge.multicart ? "YES" : "NO");
}
