cmake_minimum_required(VERSION 3.20)
project(VirtualFPGA VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(raylib)


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
    src/cad/Router.cpp

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

add_executable(router_test tests/router_test.cpp)
target_link_libraries(router_test PRIVATE vfpga_core)

add_executable(flow_test tests/flow_test.cpp)
target_link_libraries(flow_test PRIVATE vfpga_core)

# Main Application
add_executable(virtual_fpga src/main.cpp src/ui/Renderer.cpp)
target_link_libraries(virtual_fpga PRIVATE vfpga_core raylib)
# Ensure we can find raylib.h
target_include_directories(virtual_fpga PRIVATE ${raylib_SOURCE_DIR}/src)

add_executable(renderer_test tests/renderer_test.cpp src/ui/Renderer.cpp)
target_link_libraries(renderer_test PRIVATE vfpga_core raylib)
target_include_directories(renderer_test PRIVATE ${raylib_SOURCE_DIR}/src)


