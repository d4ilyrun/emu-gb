add_library(
    cartridge STATIC
    cartridge.c
    memory.c
    mbc1.c
    mbc2.c
    mbc3.c
)

target_link_libraries(cartridge PRIVATE utils)
