cmake_minimum_required(VERSION 3.16)
project(VirtualFPGA VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Source files
set(CORE_SOURCES
    src/core/LogicVal.cpp
    src/core/Signal.cpp
    src/fabric/BitstreamLoader.cpp
    src/cad/Parser.cpp
    src/cad/Packer.cpp
    src/cad/Placer.cpp

)

# Create library for core logic
add_library(vfpga_core ${CORE_SOURCES})
target_include_directories(vfpga_core PUBLIC src)

# Test executable
add_executable(vfpga_test tests/main_test.cpp)
target_link_libraries(vfpga_test PRIVATE vfpga_core)

add_executable(parser_test tests/parser_test.cpp)
target_link_libraries(parser_test PRIVATE vfpga_core)

add_executable(packer_test tests/packer_test.cpp)
target_link_libraries(packer_test PRIVATE vfpga_core)

add_executable(placer_test tests/placer_test.cpp)
target_link_libraries(placer_test PRIVATE vfpga_core)
