cmake_minimum_required(VERSION 3.15)
project(BixsBundleBash)

# Build Configuration
if(NOT ANDROID)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform Detection
if(ANDROID)
    message(STATUS "Building for Android")
    add_definitions(-DPLATFORM_ANDROID)
    set(PLATFORM_ANDROID TRUE)

    set(PLATFORM "Android" CACHE STRING "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
else()
    message(STATUS "Building for Desktop")
    add_definitions(-DPLATFORM_DESKTOP)
    set(PLATFORM_DESKTOP TRUE)

    set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "" FORCE)
endif()

# Dependencies
include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/5.5.tar.gz
)
FetchContent_Declare(
    flecs
    URL https://github.com/SanderMertens/flecs/archive/v4.0.5.tar.gz
)
FetchContent_Declare(
    FastNoiseLite
    URL https://github.com/Auburn/FastNoiseLite/archive/v1.1.1.tar.gz
)
FetchContent_Declare(
    micropather
    GIT_REPOSITORY https://github.com/leethomason/MicroPather.git
    GIT_TAG master
)

FetchContent_MakeAvailable(raylib flecs FastNoiseLite micropather)

if(NOT TARGET micropather)
    add_library(micropather STATIC 
        ${micropather_SOURCE_DIR}/micropather.cpp
    )
    target_include_directories(micropather PUBLIC 
        ${micropather_SOURCE_DIR}
    )
    target_compile_options(micropather PRIVATE -w)
endif()

if(TARGET raylib)
    target_compile_options(raylib PRIVATE -w)
    get_target_property(RAYLIB_INCLUDE_DIRS raylib INTERFACE_INCLUDE_DIRECTORIES)
    if(RAYLIB_INCLUDE_DIRS)
        target_include_directories(raylib SYSTEM INTERFACE ${RAYLIB_INCLUDE_DIRS})
    endif()
endif()

if(TARGET flecs)
    target_compile_options(flecs PRIVATE -w)
    get_target_property(FLECS_INCLUDE_DIRS flecs INTERFACE_INCLUDE_DIRECTORIES)
    if(FLECS_INCLUDE_DIRS)
        target_include_directories(flecs SYSTEM INTERFACE ${FLECS_INCLUDE_DIRS})
    endif()
endif()

if(TARGET flecs_static)
    target_compile_options(flecs_static PRIVATE -w)
endif()

# Target Configuration
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

if(PLATFORM_ANDROID)
    find_library(native-app-glue-lib android_native_app_glue)
    if(NOT native-app-glue-lib)
        set(NATIVE_APP_GLUE ${ANDROID_NDK}/sources/android/native_app_glue)
        add_library(native_app_glue STATIC ${NATIVE_APP_GLUE}/android_native_app_glue.c)
        target_include_directories(native_app_glue PUBLIC ${NATIVE_APP_GLUE})
        set(native-app-glue-lib native_app_glue)
    endif()

    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

    find_library(log-lib log)
    find_library(android-lib android)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        raylib
        flecs::flecs
        micropather
        ${native-app-glue-lib}
        ${android-lib}
        ${log-lib}
        EGL
        GLESv2
        OpenSLES
        atomic
    )

    target_compile_options(${PROJECT_NAME} PRIVATE -O2)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-u ANativeActivity_onCreate"
    )
else()
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib flecs::flecs micropather)
endif()

if(ANDROID)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-braced-scalar-init -Wno-error=missing-field-initializers -Wextra -Werror)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-braced-scalar-init -Wno-error=missing-designated-field-initializers -Wextra -Werror)
endif()

# Common Configuration
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${fastnoiselite_SOURCE_DIR}/Cpp
    ${micropather_SOURCE_DIR}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
