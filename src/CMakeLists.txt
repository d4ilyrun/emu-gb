add_library(
    utils STATIC
    log.c
    options.c
    )

add_subdirectory(cpu)
add_subdirectory(ppu)
add_subdirectory(cartridge)

