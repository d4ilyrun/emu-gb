add_library(gui STATIC 
    sdl.c
)

target_link_libraries(gui PRIVATE utils SDL2)
