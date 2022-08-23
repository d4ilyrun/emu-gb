find_package(GTest REQUIRED)
include(GoogleTest)
include(CTest)

enable_testing()

function(NewTest)

    cmake_parse_arguments(
        TEST
        ""
        "NAME"
        "SRCS;DEPS"
        ${ARGN}
    )

    set(test "${TEST_NAME}_test")
    add_executable("${test}" ${TEST_SRCS})
    target_link_libraries("${test}" PRIVATE ${TEST_DEPS} gtest gtest_main)
    add_test(NAME "${test}" COMMAND "${test}" WORKING_DIRECTORY "./tests")
    gtest_discover_tests("${test}")

endfunction()

# TODO: use subfolder

NewTest(NAME "registers" SRCS "src/cpu/registers.cc" DEPS cpu)
NewTest(NAME "rom" SRCS "src/cartridges/rom.cc" DEPS cartridge cpu)
NewTest(NAME "mbc1" SRCS "src/cartridges/mbc1.cc" DEPS cartridge cpu)
NewTest(NAME "mbc2" SRCS "src/cartridges/mbc2.cc" DEPS cartridge cpu)
