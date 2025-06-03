cmake_minimum_required(VERSION 3.15)
project(BixsBundleBash)

set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "" FORCE)

add_subdirectory(external/raylib)
add_subdirectory(external/flecs)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} raylib flecs)
target_include_directories(${PROJECT_NAME} PRIVATE src)


