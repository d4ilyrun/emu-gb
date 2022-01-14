#pragma once

#include "utils/types.h"

#define MEMORY_START            0x0000
#define ROM_BANK                0x4000
#define ROM_BANK_SWITCHABLE     0x8000
#define VIDEO_RAM               0xA000
#define EXTERNAL_RAM            0xC000
#define WORK_RAM                0xD000
#define WORK_RAM_SWITCHABLE     0xE000
#define RESERVED_ECHO_RAM       0xFE00
#define OAM                     0xFEA0
#define RESERVED_UNUSED         0xFF00
#define IO_PORTS                0xFF80
#define CPU_HIGH_RAM            0xFFFF
#define INTERRUPT_ENABLE_FLAGS  0xFFFF

void write_memory(u16 address, u8 val);
void write_memory_16bit(u16 address, u16 val);

u8 read_memory(u16 address);
u16 read_memory_16bit(u16 address);
