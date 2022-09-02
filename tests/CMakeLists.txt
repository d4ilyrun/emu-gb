find_package(GTest REQUIRED)
include(GoogleTest)
include(CTest)

enable_testing()

function(NewTest)

    cmake_parse_arguments(
        TEST
        ""
        "NAME;PREFIX"
        "SRCS;DEPS"
        ${ARGN}
    )

    set(test "${TEST_NAME}_test")
    set(TEST_NAME "${TEST_PREFIX}/${TEST_NAME}")

    add_executable(${test} ${TEST_SRCS})
    add_custom_command(TARGET ${test} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_PREFIX})
    target_link_libraries(${test} PRIVATE ${TEST_DEPS} gtest gtest_main)
    set_target_properties(${test} PROPERTIES OUTPUT_NAME ${TEST_NAME})
    add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})

    gtest_discover_tests("${test}")

endfunction()

# CPU
NewTest(NAME "registers" PREFIX "cpu" SRCS "src/cpu/registers.cc" DEPS cpu)
NewTest(NAME "interrupts" PREFIX "cpu" SRCS "src/cpu/interrupt.cc" "../src/cpu/timer.c" DEPS cpu cartridge)
NewTest(NAME "timer" PREFIX "cpu" SRCS "src/cpu/timer.cc" DEPS cpu cartridge)

# CPU Instruction set
NewTest(NAME "ld" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/load/register.cc"
             "src/cpu/instructions/load/immediate.cc"
             "src/cpu/instructions/load/relative.cc"
             "src/cpu/instructions/arithmetic/ld.cc"
        DEPS cpu cartridge)

NewTest(NAME "ldd" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/load/ldd.cc"
        DEPS cpu cartridge)

NewTest(NAME "ldi" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/load/ldi.cc"
        DEPS cpu cartridge)

NewTest(NAME "push" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/load/push.cc"
        DEPS cpu cartridge)

NewTest(NAME "pop" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/load/pop.cc"
        DEPS cpu cartridge)

NewTest(NAME "add" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/add.cc"
        DEPS cpu cartridge)

NewTest(NAME "adc" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/adc.cc"
        DEPS cpu cartridge)

NewTest(NAME "sbc" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/sbc.cc"
        DEPS cpu cartridge)

NewTest(NAME "sub" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/sub.cc"
        DEPS cpu cartridge)

NewTest(NAME "dec" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/dec.cc"
        DEPS cpu cartridge)

NewTest(NAME "inc" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/inc.cc"
        DEPS cpu cartridge)

NewTest(NAME "and" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/and.cc"
        DEPS cpu cartridge)

NewTest(NAME "or" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/or.cc"
        DEPS cpu cartridge)

NewTest(NAME "xor" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/xor.cc"
        DEPS cpu cartridge)

NewTest(NAME "cp" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/cp.cc"
        DEPS cpu cartridge)

NewTest(NAME "daa" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/daa.cc"
        DEPS cpu cartridge)

NewTest(NAME "cpl" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/logical/cpl.cc"
        DEPS cpu cartridge)

NewTest(NAME "sra" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/shift/sra.cc"
        DEPS cpu cartridge)

NewTest(NAME "rla" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/shift/rla.cc"
        DEPS cpu cartridge)

NewTest(NAME "bit" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/cb/bit.cc"
        DEPS cpu cartridge)


# CARTRIDGES
NewTest(NAME "rom"  PREFIX "cartridge" SRCS "src/cartridges/rom.cc" DEPS cartridge cpu)
NewTest(NAME "mbc1" PREFIX "cartridge" SRCS "src/cartridges/mbc1.cc" DEPS cartridge cpu)
NewTest(NAME "mbc2" PREFIX "cartridge" SRCS "src/cartridges/mbc2.cc" DEPS cartridge cpu)
NewTest(NAME "mbc3" PREFIX "cartridge" SRCS "src/cartridges/mbc3.cc" "${PROJECT_SOURCE_DIR}/src/cartridge/mbc3.c" DEPS cartridge cpu)
