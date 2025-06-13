cmake_minimum_required(VERSION 3.15)
project(BixsBundleBash)

# Build Configuration
set(CMAKE_BUILD_TYPE Debug)
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

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

# Target Configuration
set(SRC_FILES src/main.cpp)

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
        ${native-app-glue-lib}
        ${android-lib}
        ${log-lib}
        EGL
        GLESv2
        OpenSLES
        atomic
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-u ANativeActivity_onCreate"
    )
else()
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
endif()

# Common Configuration
target_include_directories(${PROJECT_NAME} PRIVATE src)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
