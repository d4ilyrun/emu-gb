add_library(
    utils STATIC
    log.c
    )

add_subdirectory(CPU)
add_subdirectory(cartridge)

