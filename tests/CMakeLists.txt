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

# TODO: use subfolder

# CPU
NewTest(NAME "registers" PREFIX "cpu" SRCS "src/cpu/registers.cc" DEPS cpu)

# CPU Instruction set
NewTest(NAME "ld" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/ld/register.cc"
             "src/cpu/instructions/ld/immediate.cc"
             "src/cpu/instructions/ld/relative.cc"
        DEPS cpu cartridge)

NewTest(NAME "add" PREFIX "cpu/instructions"
        SRCS "src/cpu/instructions/arithmetic/add.cc"
        DEPS cpu cartridge)

# CARTRIDGES
NewTest(NAME "rom"  PREFIX "cartridge" SRCS "src/cartridges/rom.cc" DEPS cartridge cpu)
NewTest(NAME "mbc1" PREFIX "cartridge" SRCS "src/cartridges/mbc1.cc" DEPS cartridge cpu)
NewTest(NAME "mbc2" PREFIX "cartridge" SRCS "src/cartridges/mbc2.cc" DEPS cartridge cpu)
NewTest(NAME "mbc3" PREFIX "cartridge" SRCS "src/cartridges/mbc3.cc" "${PROJECT_SOURCE_DIR}/src/cartridge/mbc3.c" DEPS cartridge cpu)
