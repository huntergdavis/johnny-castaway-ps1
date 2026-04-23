cmake_minimum_required(VERSION 3.21)

# Find PSn00bSDK first (required before project())
if(NOT DEFINED ENV{PSN00BSDK})
    message(FATAL_ERROR "PSN00BSDK environment variable not set")
endif()

# Set toolchain file BEFORE project()
list(APPEND CMAKE_MODULE_PATH "$ENV{PSN00BSDK}/lib/libpsn00b/cmake")
include(sdk)

# Project configuration
project(
    JohnnyReborn
    LANGUAGES C
    VERSION 1.0.0
)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags matching the original project
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -DPS1_BUILD -ffreestanding")

# Source files - using PS1-specific implementations for graphics/sound/events
# All translation units live under src/.
set(SOURCES
    src/jc_reborn.c
    src/utils.c
    src/uncompress.c
    src/resource.c
    src/dump.c
    src/story.c
    src/walk.c
    src/calcpath.c
    src/ads.c
    src/foreground_pilot.c
    src/ttm.c
    src/island.c
    src/bench.c
    src/config.c
    # PS1-specific implementations
    src/graphics_ps1.c
    src/sound_ps1.c
    src/events_ps1.c
    src/cdrom_ps1.c
    src/ps1_debug.c
    # LibC stubs for missing functions
    src/ps1_stubs.c
)

# Create PS1 executable
# Note: GPREL = use GP-relative addressing for faster data access
psn00bsdk_add_executable(jcreborn GPREL ${SOURCES})

# Include directories
# - src/ for the main translation units' own headers
# - project root so `#include "config/ps1/bootmode_embedded.h"` still resolves
target_include_directories(jcreborn
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link PSn00bSDK libraries explicitly
target_link_libraries(jcreborn PRIVATE
    psxgpu
    psxgte
    psxspu
    psxcd
    c
)

# CD image creation is now handled by make-cd-image.sh script
# (POST_BUILD mkpsxiso was causing hangs when trying to overwrite existing files)

# Install target
install(TARGETS jcreborn)
