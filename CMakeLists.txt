cmake_minimum_required(VERSION 3.15)
project(BixsBundleBash)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "" FORCE)

find_package(flecs CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE raylib flecs::flecs glfw)
target_include_directories(${PROJECT_NAME} PRIVATE src)
