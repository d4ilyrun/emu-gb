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
     * Bit 7 - LCD and PPU enable               0=Off, 1=On
     * Bit 6 - Window tile map area             0=9800-9BFF, 1=9C00-9FFF
     * Bit 5 - Window enable                    0=Off, 1=On
     * Bit 4 - BG and Window tile data area     0=8800-97FF, 1=8000-8FFF
     * Bit 3 - BG tile map area                 0=9800-9BFF, 1=9C00-9FFF
     * Bit 2 - OBJ size                         0=8x8, 1=8x16
     * Bit 1 - OBJ enable                       0=Off, 1=On
     * Bit 0 - BG and Window enable/priority	0=Off, 1=On
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

/// LCD LCDC REGISTER RELATED MACROS

#define LCD_ENABLED(_lcd)        BIT((_lcd).lcdc, 7)
#define LCD_WINDOW_ENABLED(_lcd) BIT((_lcd).lcdc, 5)

#define LCD_WINDOW_TILE_MAP(_lcd)  (0x9800 | (BIT((_lcd).lcdc, 6) << 10))
#define LCD_WINDOW_TILE_DATA(_lcd) (0x8000 | ((~BIT((_lcd).lcdc, 4)) << 11))

#define LCD_BG_TILE_MAP(_lcd)  (0x9800 | (BIT((_lcd).lcdc, 3) << 10))
#define LCD_BG_TILE_DATA(_lcd) LCD_BG_TILE_DATA(_lcd)

#define LCD_OBJ_SIZE(_lcd)   ((1 << 6) | (BIT((_lcd).lcdc, 2) << 7))
#define LCD_OBJ_ENABLE(_lcd) BIT((_lcd).lcdc, 1)

#define LCD_BG_WINDOW_ENABLED(_lcd)  BIT((_lcd).lcdc, 0)
#define LCD_BG_WINDOW_PRIORITY(_lcd) BIT((_lcd).lcdc, 0)

/// LCD STAT REGISTER RELATED MACROS

#define LCD_STAT_LYC_IRS(_lcd)    BIT((_lcd).stat, 6) ///< LYC=LY Interrupt source
#define LCD_STAT_OAM_IRS(_lcd)    BIT((_lcd).stat, 5) ///< Interrupt source
#define LCD_STAT_VBLANK_IRS(_lcd) BIT((_lcd).stat, 4) ///< Interrupt source
#define LCD_STAT_HBLANK_IRS(_lcd) BIT((_lcd).stat, 3) ///< Interrupt source
#define LCD_STAT_LYC_FLAG(_lcd)   BIT((_lcd).stat, 2) ///< LYC=LY Flag
#define LCD_STAT_MODE_FLAG(_lcd)  ((_lcd).stat & 0x3) ///< Mode flag

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
typedef enum {
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

/**
 * \enum lcd_mode
 * \brief The different states the LCD can be in
 */
typedef enum lcd_mode {
    MODE_HBLANK = 0,
    MODE_VBLANK = 1,
    MODE_OAM = 2,
    MODE_TRANSFER = 3
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

/**
 * \function lcd_increment_ly
 * \brief Increment LY register (readonly otherwise)
 *
 * \warning The value inside LY can only be between [0-153] and will loop back to
 * 0 when overpassing this limit.
 */
void lcd_increment_ly();
