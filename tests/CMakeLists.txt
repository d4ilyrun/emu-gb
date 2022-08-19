find_package(GTest REQUIRED)
include(GoogleTest)

enable_testing()

function(TestFeature feature file libs)
    add_executable("test_${feature}" ${file})
    target_link_libraries("test_${feature}" PRIVATE ${libs} gtest gtest_main)
    add_test(NAME "test_${feature}" COMMAND "${feature}")
    gtest_discover_tests("test_${feature}")
endfunction()

TestFeature("registers" "src/cpu/registers.cc" cpu)
