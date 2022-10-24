#pragma once

#include "utils/types.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// TODO: ADD fucking commentsbut i'm too lazy rn

void gui_init(const char *path);
void gui_deinit();
void *gui_main(void *arg);

void gui_push_pixel(u32 pixel, u8 x, u8 y);
