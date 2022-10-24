#include "gui/sdl.h"
#include "ppu/lcd.h"
#include "utils/log.h"

u32 video_buffer[LCD_HEIGHT][LCD_WIDTH];

void gui_push_pixel(u32 pixel, u8 x, u8 y)
{
    if (pixel)
        log_info("[%x][%x] = %lX", x, y, pixel);
    video_buffer[y][x] = pixel;
}
