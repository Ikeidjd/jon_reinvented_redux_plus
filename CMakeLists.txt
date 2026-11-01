cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

project(jon VERSION 1.0)

file(GLOB_RECURSE SRC_FILES src/*.c)
add_executable(jon ${SRC_FILES})

target_include_directories(jon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
