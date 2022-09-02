add_library(
    cpu STATIC
    cpu.c
    instruction.c
    instruction_cb.c
    instruction_display.c
    instruction_fetch.c
    interrupt.c
    memory.c
    timer.c
    ../io.c
    )
