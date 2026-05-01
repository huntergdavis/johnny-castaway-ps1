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

option(PS1_PERF_DEEP_TRACE "Include large, diagnostic-only PS1 perf trace counters" OFF)

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
    src/foreground_pilot.c
    src/ps1_perf.c
    src/island.c
    # PS1-specific implementations
    src/graphics_ps1.c
    src/sound_ps1.c
    src/events_ps1.c
    src/cdrom_ps1.c
    src/ps1_debug.c
    src/pause_menu.c
    # Closed captions — text + scene-to-ADS map + runtime hooks.
    src/ps1_captions.c
    # SPI driver — direct controller polling, lifted from PSn00bSDK
    # examples/io/pads/ (MPL licensed, (C) 2021 spicyjpeg).
    # Replaces the BIOS pad driver, which never auto-polls in our
    # PSn00bSDK 0.24 + DuckStation environment (verified empirically).
    src/spi.c
    # Memcard persistence for pause-menu settings.
    src/memcard.c
    # Holiday table + date-algorithm core (36 holidays, codegen from holidays.yml).
    src/holidays.c
    src/holidays_table.c
    # Walk subsystem — story-loop walks between scenes + freeplay walking.
    # walk.c + walk_data.h came from upstream jc_reborn; calcpath.c is
    # the small Dijkstra over the 6-spot graph. walk_render.c is the
    # shared draw kernel both story-loop and freeplay call. See
    # docs/ps1/walk-implementation-plan.md.
    src/walk.c
    src/calcpath.c
    src/walk_render.c
    src/walk_pilot.c
    src/scene_freeplay.c
    # LibC stubs for missing functions
    src/ps1_stubs.c
)

# Size-optimize only translation units that have passed the exact fishing1
# gate. Most hot files stay at the SDK default because code phase is fragile.
set_source_files_properties(
    src/jc_reborn.c
    src/utils.c
    src/island.c
    src/ps1_debug.c
    src/foreground_pilot.c
    src/holidays.c
    src/resource.c
    src/sound_ps1.c
    src/events_ps1.c
    src/pause_menu.c
    src/ps1_stubs.c
    src/ps1_captions.c
    src/memcard.c
    src/scene_freeplay.c
    PROPERTIES COMPILE_OPTIONS "-Os"
)

# Create PS1 executable
# Note: GPREL = use GP-relative addressing for faster data access
psn00bsdk_add_executable(jcreborn GPREL ${SOURCES})

# Keep PS1 release builds tight: each function/data object gets its own
# section so the linker can drop unused legacy support paths.
target_compile_options(jcreborn PRIVATE
    -ffunction-sections
    -fdata-sections
)
target_link_options(jcreborn PRIVATE
    -Wl,--gc-sections
)
if(PS1_PERF_DEEP_TRACE)
    target_compile_definitions(jcreborn PRIVATE PS1_PERF_DEEP_TRACE=1)
endif()

# Include directories
# - src/ for the main translation units' own headers
# - project root so `#include "config/ps1/bootmode_embedded.h"` still resolves
target_include_directories(jcreborn
    BEFORE PRIVATE
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
