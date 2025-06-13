cmake_minimum_required(VERSION 3.15)
project(BixsBundleBash)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.0.5
)

FetchContent_MakeAvailable(raylib flecs)
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE raylib flecs::flecs)
target_include_directories(${PROJECT_NAME} PRIVATE src)