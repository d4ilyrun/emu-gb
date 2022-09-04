add_library(ppu STATIC ppu.c)

target_link_libraries(ppu PRIVATE utils)
