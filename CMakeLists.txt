cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c23 -fno-strict-aliasing -Wall -Wextra -Wpedantic -Werror -g -O0")

project(jon VERSION 1.0)

file(GLOB_RECURSE SRC_FILES src/*.c)
add_executable(jon ${SRC_FILES})

target_include_directories(jon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
