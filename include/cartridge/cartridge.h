#pragma once

/**
 * \file cartridge.cc
 * \author Léo DUBOIN
 * \brief Load, read and write to a gameboy cartridge.
 */

#include "utils/types.h"

/**
 * \brief 0x0104 - 0x0133 - Nintendo logo
 *
 * These 48 bytes represent the Nintendo logo that is shown when the Game Boy is
 * powered on (the ones that appear corrupted if the cartridge is not inserted
 * correctly, for example).
 */
static u8 g_nintendo_logo[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
    0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

/**
 * \struct game_info
 * \brief Game information
 */
struct game_info {
    char game_title[11];
    char manufacturer_code[4]; ///< 4 character uppercase code

    /// Flag used to enable GBC functions in CGB/AGB/AGS
    u8 cbg_flag;
};

/**
 * \struct cartridge_header
 *
 * The Cartridge Header (0x0100 - 0x014F)
 *
 * The area at 0100h-014Fh of the ROM is reserved for some special information.
 * It holds information about:
 * - the cartridge's starting protocol
 * - the cartridge's type
 * - the cartridge's rom/ram size
 * - the game's version and ditributor
 * - some intergrity checks
 */
struct cartridge_header {
    /**
     * The cartridge starting protocol. Executed when lauching the game.\n
     * Usually a \c nop follow by a <tt>jp 150h</tt>.
     */
    u8 start_vector[4];

    u8 nintendo_logo[sizeof(g_nintendo_logo)];
    struct game_info game_info; ///< \see game_info

    /**
     * Indicates the publisher. 2 character uppercase.
     * Only used by games released after the SGB, set to 0x014B by default;
     */
    u16 new_license_code;

    u8 sgb_flag; ///< Indicates wether the game supports SGB functions
    u8 type;     ///< The type of the cartridge. \see cartridge_type
    u8 rom_size; ///< ROM size: 32 << \c value
    u8 ram_size; ///< Specifies RAM size if any.
    u8 dst_code; ///< Set to 0x00 if sold to Japan, else set to 0x01

    /**
     * Indicates the publishing company.
     * If set to 0x33, use the \c new_license_code header instead.
     */
    u8 old_license_code;

    u8 rom_version; ///< In case multiple versions of the game were released.
    u8 header_checksum;  ///< Checksum to ensure the intergrity of the ROM.
    u16 global_checksum; ///< Not verified in the Game Boy
};

#define ROM_MAX_FILENAME_SIZE 1024

/**
 * \struct cartridge
 * \brief Information about the cartridge.
 *
 * The cartridge ROM is loaded during startup and should never be modified. \n
 * Use the memory API to interact with it instead.
 *
 * \see write_memory read_memory
 */
struct cartridge {
    char filename[ROM_MAX_FILENAME_SIZE]; ///< Path to the rom file.

    /**
     * Some cartridges include more than one game.\n
     * Those cartriges have a slightly different banking mechanism.
     *
     * \see read_mbc1
     */
    bool multicart;

    /**
     * Actual size of the game's ROM. \n
     * Should be equivalent to 32 < \c rom_size header.
     */
    u32 rom_size;

    /**
     * Actual size of the game's RAM. \n
     * Depends on the \c ram_size inside the header
     */
    u32 ram_size;

    /**
     * The actual content of the game's ROM. \n
     * \warning ROM stands for Read-Only memory, it should NEVER be modified.
     */
    u8 *rom;

    /**
     * The game's RAM.
     */
    u8 *ram;
};

/**
 * \brief The different types of cartridge
 *
 * The enum values correspond to the cartridge's \c type header value.
 * For example, all rom versions lesser or equal to MBC1 will be considered of
 * type MBC1.
 */
typedef enum cartridge_type {
    ROM_ONLY = 0x0,
    MBC1 = 0x03,
    MBC2 = 0x06,
    MBC3 = 0x13,
    MBC5 = 0x1E,
    MBC6 = 0x20,
    MBC7 = 0x22,
} cartridge_type;

/// The game cartridge loaded with the emulator
extern struct cartridge g_cartridge;

/// Find the cartridge's header and cast to the correct type
#define CARTRIDGE_HEADER_START (0x100)
#define HEADER(_cart) \
    ((struct cartridge_header *)((_cart).rom + CARTRIDGE_HEADER_START))

/**
 * \brief load a cartridge in memory.
 *
 * The cartridge will be loaded in the global cartridge variable.
 *
 * \see cartridge
 *
 * \return wether the cartridge has been loaded sucessfully.
 */
bool load_cartridge(char *path);

/**
 * \brief Print the cartridge's information.
 */
void cartridge_info();

/**
 * \copydoc read_memory
 */
u8 read_cartridge(u16 address);

/**
 * \copydoc read_memory_16bit
 */
u16 read_cartridge_16bit(u16 address);

/**
 * \copydoc write_memory
 */
void write_cartridge(u16 address, u8 data);

/**
 * \copydoc write_memory_16bit
 */
void write_cartridge_16bit(u16 address, u16 data);
