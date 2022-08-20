find_package(GTest REQUIRED)
include(GoogleTest)
include(CTest)

enable_testing()

function(TestFeature feature file libs)
    set(feature "${feature}_test")
    add_executable("${feature}" ${file})
    target_link_libraries("${feature}" PRIVATE ${libs} gtest gtest_main)
    add_test(NAME "${feature}" COMMAND "${feature}" WORKING_DIRECTORY "./tests")
    gtest_discover_tests("${feature}")
endfunction()

TestFeature("registers" "src/cpu/registers.cc" cpu)
