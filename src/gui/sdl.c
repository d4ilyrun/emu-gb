#include "gui/sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

#include "cpu/cpu.h"
#include "ppu/lcd.h"
#include "utils/error.h"
#include "utils/log.h"

extern u32 video_buffer[LCD_HEIGHT][LCD_WIDTH];

struct gui {
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Surface *screen;
    SDL_Texture *texture;
};

static struct gui gui;

void gui_init(const char *path) {
    assert_msg(SDL_Init(SDL_INIT_VIDEO) == 0, "SDL: Couldn't init SDL: %s", SDL_GetError());

    // Create the window where we will display the game content
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gui.window, &gui.renderer);
    gui.window = SDL_CreateWindow(path, 0, 0, 1000, 1000, 0);
    assert_msg(gui.renderer != NULL, "SDL: Couldn't create renderer: %s", SDL_GetError());
    assert_msg(gui.window != NULL, "SDL: Couldn't create window: %s", SDL_GetError());

    // Fill screen with black
    gui.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                      0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    // Set texture as "modified frequently"
    gui.texture = SDL_CreateTexture(gui.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                    SCREEN_WIDTH, SCREEN_HEIGHT);
}

void gui_deinit() {
    SDL_DestroyRenderer(gui.renderer);
    SDL_Quit();
}

#define loop() while(1)

static void gui_update_screen()
{
    static u8 scale_width = (SCREEN_WIDTH / LCD_WIDTH);
    static u8 scale_height = (SCREEN_HEIGHT / LCD_HEIGHT);

    SDL_Rect pixel;

    pixel.w = scale_width;
    pixel.h = scale_height;

    for (u8 i = 0; i < LCD_HEIGHT; ++i) {
        pixel.y = i * scale_height;
        for (u8 j = 0; j < LCD_WIDTH; ++j) {
            u32 raw_pixel = video_buffer[i][j];
            pixel.x = j * scale_height;
            SDL_FillRect(gui.screen, &pixel, raw_pixel);
        }

    }

    //SDL_UpdateTexture(gui.texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(gui.renderer);
    SDL_RenderCopy(gui.renderer, gui.texture, NULL, NULL);
    SDL_RenderPresent(gui.renderer);
}

void *gui_main(void *arg)
{
    (void) arg; // unused

    SDL_Event event;

    loop() {
        SDL_PollEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            gui_deinit();
            cpu.is_running = false;
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_LEFT)
                log_info("Pressed LEFT");
            else if (event.key.keysym.sym == SDLK_RIGHT)
                log_info("Pressed RIGHT");
            break;


        default:
            (void) gui_update_screen();
            continue;
        };
    }
}
