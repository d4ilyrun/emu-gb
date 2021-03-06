#include "cartridge/cartridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartridge/memory.h"
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
    u8 bank2 = chip_registers.bank_2;
    chip_registers.bank_1 = 0;
    chip_registers.mode = true;

    // Loop through all four possiblbbe BANK2 values
    for (chip_registers.bank_2 = 0b00; chip_registers.bank_2 <= 0b11;
         ++chip_registers.bank_2) {
        // Look for the nintendo logo
        printf("NEW BANK: %d\n", chip_registers.bank_2);
        for (size_t i = 0; i < sizeof(nintendo_logo); ++i) {
            printf("%x = %x, ", read_cartridge(0x0104 + i), nintendo_logo[i]);
            if (read_cartridge(0x0104 + i) != nintendo_logo[i]) {
                nb_games -= 1;
                break;
            }
        }
        printf("\n");
    }

    printf("%d\n", nb_games);
    cartridge.multicart = nb_games;
    chip_registers.bank_2 = bank2;
    chip_registers.mode = false; // Always initialized at false
    return cartridge.multicart;
}

bool load_cartridge(char *path)
{
    FILE *rom = fopen(path, "r");

    if (rom == NULL) {
        perror("Failed to load cartridge");
        return false;
    }

    // TODO: check if filename is too long !!!
    strcpy(cartridge.filename, path);

    // Get rom_size and allocate enough space to store the cartridge's rom
    fseek(rom, 0, SEEK_END);
    cartridge.rom_size = ftell(rom);
    cartridge.rom = malloc(cartridge.rom_size);
    cartridge.multicart = false;

    rewind(rom);
    fread(cartridge.rom, 1, cartridge.rom_size, rom);

    if (verify_header_checksum(cartridge)) {
        fputs("Failed to load cartridge: Invalid checksum.", stderr);
        return false;
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
    printf("\tMulticart : %s\n", cartridge.multicart ? "YES" : "NO");
    putchar('\n');
}
