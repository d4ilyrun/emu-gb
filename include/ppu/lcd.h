#pragma once

#include "utils/macro.h"
#include "utils/types.h"

/// Width of the LCD screen in pixels
#define LCD_WIDTH 160

/// Height of the LCD screen in pixels
#define LCD_HEIGHT 160

struct lcd {
    /**
     * LCDC is the main LCD Control register. Its bits toggle what elements are
     * displayed on the screen, and how.
     *
     * Located at 0xFF40.
     *
     * \ref https://gbdev.io/pandocs/LCDC.html#ff40---lcdc-lcd-control-rw
     */
    u8 lcdc;

    /*
     * STAT mode register. (0xFF41)
     *
     * Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
     * Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
     * Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
     * Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
     * Bit 2 - LYC=LY Flag (Read Only)              (0=Different, 1=Equal)
     * Bit 1-0 - Mode Flag (Read Only)              (Mode 0-3)
     *   0: HBlank
     *   1: VBlank
     *   2: Searching OAM
     *   3: Transferring Data to LCD Controller
     */
    u8 stat;

    u8 scy; /// Scroll Y, 0xFF42
    u8 scx; /// Scroll X, 0xFF43
    u8 ly;  /// Y coordinate, 0xFF44
    u8 lyc; /// LY compare, 0xFF45

    u8 dma; // 0xFF46

    /// Monochrome (non-CGB only)
    struct {
        u8 bgp;    /// BG palette data, 0xFF47
        u8 obp[2]; /// OBJ palette data, 0xFF48-9
    } dmg;

    u8 wy; /// Window Y position, 0xFF4A
    u8 wx; /// Window X position, 0xFF4B

    // TODO: GameBoy color
    /// Color (CGB only)
    struct {
        u8 bgpi; /// BG color palette index, 0xFF68
        u8 bgpd; /// BG color palette data, 0xFF69
        u8 obpi; /// OBJ color palette index, 0xFF6A
        u8 obpd; /// OBJ color palette data, 0xFF6B
    } cgb_colors;
};

/**
 * \function get_lcd
 * \brief Get a pointer to the current LCD variable (registers and status)
 * \return A \c const pointer to the \c static LCD struct inside \c lcd.c
 */
const struct lcd *get_lcd();

/**
 * \function init_lcd
 * \brief Initialize the LCD's control registers and palettes
 */
void init_lcd();

/**
 * \function read_lcd
 * \brief read a 8bit value from the LCD screen's registers.
 *
 * \param address 16bit memory address between 0xFF40-0xFF4A and 0xFF68-0xFF6A
 * \see read_memory
 */
u8 read_lcd(u16 address);

/**
 * \function write_lcd
 * \brief write an 8bit value to the LCD screen's registers.
 *
 * \param address 16bit memory address between 0xFF40-0xFF4A and 0xFF68-0xFF6A
 * \param val 8bit value
 * \see write_memory
 */
void write_lcd(u16 address, u8 value);

typedef u32 shade;

/**
 * \enum palette_name
 *
 * Color palettes:
 *  - The DMG has 1 palette of 4 gray shades for backgrounds and 2
 *    palettes of 3 gray shades for sprites (color 0 is transparent).
 *  - The GBC has 8 palettes of 4 colors for backgrounds and 8 palettes of 3
 *    colors for sprites (color 0 is transparent).
 */
typedef enum
{
    // DMG
    BG_PALETTE = 0,
    SPRITE_PALETTE_0,
    SPRITE_PALETTE_1,
    INVALID
} palette_name;

/**
 * \function lcd_get_palette
 * \brief Return a palette according to a given name
 *
 * A palette is a collection of 4 shades.
 *
 * \param palette The name of the palette
 * \return The corresponding palette
 *
 * \see shade
 * \see palette_name
 */
shade *lcd_get_palette(palette_name palette);

/// LCD STAT REGISTER RELATED MACROS

#define LCD_STAT_LYC_IRS(_lcd) BIT((_lcd).stat), 6)     /// LYC=LY Interrupt source
#define LCD_STAT_OAM_IRS(_lcd) BIT((_lcd).stat), 5)     /// OAM Interrupt source
#define LCD_STAT_VBLANK_IRS(_lcd) BIT((_lcd).stat), 4)  /// VBlank Interrupt source
#define LCD_STAT_HBLANK_IRS(_lcd) BIT((_lcd).stat), 3)  /// HBlank Interrupt source
#define LCD_STAT_LYC_FLAG(_lcd) BIT((_lcd).stat, 2)     /// LYC=LY Flag
#define LCD_STAT_MODE_FLAG(_lcd) ((_lcd).stat & 0x3)    /// Mode flag

/**
 * \enum lcd_mode
 * \brief The different states the LCD can be in
 */
typedef enum lcd_mode
{
    MODE_HBLANK = 0,
    MODE_VBLANK,
    MODE_OAM,
    MODE_TRANSFER
} lcd_mode;

/**
 * \function lcd_set_mode
 * \brief Set the state of the lcd
 * \param mode The new state of the lcd
 * \see lcd_mode
 */
void lcd_set_mode(lcd_mode mode);

/**
 * \function lcd_get_mode
 * \brief Get the current state of the lcd
 * \return The current state of the lcd
 * \see lcd_mode
 */
lcd_mode lcd_get_mode();
