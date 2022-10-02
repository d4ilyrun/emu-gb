add_library(ppu STATIC 
    ppu.c
    lcd.c
)

target_link_libraries(ppu PRIVATE utils)
