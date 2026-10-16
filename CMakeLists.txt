cmake_minimum_required(VERSION 3.20)
project(Nespeculator C)

# Allow a second, identically-built executable in a separate build directory
# (see tools/ppu_toolchain.cmake) so parallel work does not collide.
if(NOT NES_EXE_NAME)
    set(NES_EXE_NAME "Nespeculator")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(NES_SOURCES
    src/core/nes.c
    src/core/cart.c
    src/core/cpu.c
    src/core/cpu_ops.c
    src/core/bus.c
    src/core/ppu.c
    src/core/apu.c
    src/core/input.c
    src/core/state.c
    src/frontend/main.c
    src/frontend/hud.c
    src/frontend/render.c
    src/frontend/input_map.c
)

# ---------------------------------------------------------------- SDL3 ------
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3")
if(NOT EXISTS "${SDL3_DIR}/include/SDL3/SDL.h")
    message(FATAL_ERROR "SDL3 development files not found in third_party/SDL3")
endif()

if(WIN32)
    set(SDL3_LIB "${SDL3_DIR}/lib/x64/SDL3.lib")
    set(SDL3_DLL "${SDL3_DIR}/lib/x64/SDL3.dll")
else()
    set(SDL3_LIB "${SDL3_DIR}/lib/x64/SDL3.lib")
    set(SDL3_DLL "${SDL3_DIR}/lib/x64/SDL3.dll")
endif()

add_executable(${NES_EXE_NAME} WIN32 ${NES_SOURCES})

target_include_directories(${NES_EXE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend"
    "${SDL3_DIR}/include"
)

if(MSVC)
    target_compile_options(${NES_EXE_NAME} PRIVATE /W3 /wd4996 /utf-8 /MP)
    target_compile_definitions(${NES_EXE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
    set_property(TARGET ${NES_EXE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
else()
    target_compile_options(${NES_EXE_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

target_link_libraries(${NES_EXE_NAME} PRIVATE "${SDL3_LIB}" winmm)

# ---------------------------------------------------------------- icon ------
# artwork/nespeculator.ico (built by tools/make_icon.py) is embedded into the
# executable so the shell, taskbar and title bar use it.  The .rc is generated
# because the resource compiler needs a native, absolute path.
if(WIN32)
    set(NES_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/artwork/nespeculator.ico")
    if(EXISTS "${NES_ICON_ICO}")
        file(TO_NATIVE_PATH "${NES_ICON_ICO}" NES_ICON_RC_PATH)
        string(REPLACE "\\" "\\\\" NES_ICON_RC_PATH "${NES_ICON_RC_PATH}")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/nespeculator.rc.in"
                       "${CMAKE_CURRENT_BINARY_DIR}/nespeculator.rc" @ONLY)
        # Re-run the resource compiler whenever the icon itself changes, not
        # only when the .rc does (tools/make_icon.py rewrites the .ico).
        set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/nespeculator.rc"
                                    PROPERTIES OBJECT_DEPENDS "${NES_ICON_ICO}")
        target_sources(${NES_EXE_NAME} PRIVATE
                       "${CMAKE_CURRENT_BINARY_DIR}/nespeculator.rc")
    else()
        message(STATUS "artwork/nespeculator.ico not found: building without an embedded icon")
    endif()
endif()

# Copy the SDL3 runtime next to the executable so it can be launched directly.
add_custom_command(TARGET ${NES_EXE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_DLL}"
            "$<TARGET_FILE_DIR:${NES_EXE_NAME}>/SDL3.dll"
    COMMENT "Copying SDL3.dll next to the executable")
