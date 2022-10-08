add_library(ppu STATIC 
    ppu.c
    lcd.c
    fifo.c
)

target_link_libraries(ppu PRIVATE utils)
