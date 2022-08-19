# Download GTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
    )

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

function(TestFeature feature file libs)
    add_executable("test_${feature}" ${file})
    target_link_libraries("test_${feature}" ${libs} gtest_main)
    add_test(NAME "test_${feature}" COMMAND "${feature}")
endfunction()

TestFeature("registers" "src/cpu/registers.cc" cpu)
