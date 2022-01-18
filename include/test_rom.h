#pragma once

/*
 * This file is only useful to tests roms in /test/roms
 * I copied these tests from the "Low Leve Devel" channel on youtube.
 *
 * - https://www.youtube.com/watch?v=hrK9nc13vxo&t=641s
 */

#define TEST_CHECK 0xFF02
#define TEST_VALUE 0xFF01

#define CHECK_TRUE 0x81

void test_rom_update();
void test_rom_print();
