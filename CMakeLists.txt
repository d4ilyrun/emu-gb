CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

# SET UP PROJECT
PROJECT(
    emu-gb
    VERSION 0.0.0
    LANGUAGES C CXX
    )

option(BUILD_TESTS "Build unit tests along with the program" OFF)

# BUILD OPTIONS
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ERROR HANDLING
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if (BUILD_TESTS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUNIT_TEST")
endif()

# OPTIMISATION FLAGS
set(C_FLAGS "-Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-missing-field-initializers -Wno-unused-result")
set(OPTI_FLAGS "-O3 -UNDEBUG")
set(DEBUG_FLAGS "-D_DEBUG_LOG -DNDEBUG -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O0 -g3 -fsanitize=address")

# C COMPILATION FLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPTI_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_FLAGS}")

# C++ COMPILATION FLAGS
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPTI_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")


# SOURCE DIRECTORIES
set(CMAKE_SOURCE_DIR src/ include/)
include_directories(${CMAKE_SOURCE_DIR})

# SUBDIRECTORIES
add_subdirectory(src/)

# TESTS
if (BUILD_TESTS)
    add_subdirectory(tests/)
else ()
    message("<!> UNIT TESTS NOT BUILT <!>")
    message("-- If you want to build the unit tests use '-DBUILD_TESTS=ON'")
endif()

# PROJECT EXECUTABLE
add_executable(emu-gb src/main.c src/test_rom.c)
target_link_libraries(emu-gb PRIVATE utils cpu cartridge ppu)

# DOCUMENTATION
add_subdirectory(doc)
