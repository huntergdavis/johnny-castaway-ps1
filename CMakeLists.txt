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
    src/core/utils.c
    src/core/uncompress.c
    src/resource/resource.c
    src/foreground_pilot/foreground_pilot.c
    src/platform/ps1/ps1_perf.c
    src/scene/island.c
    # PS1-specific implementations
    src/graphics_ps1/graphics_ps1.c
    src/platform/ps1/sound_ps1.c
    src/platform/ps1/events_ps1.c
    src/platform/ps1/ps1_pad_script.c
    src/cdrom_ps1.c
    src/platform/ps1/ps1_debug.c
    src/pause_menu/pause_menu.c
    # Closed captions — text + scene-to-ADS map + runtime hooks.
    src/platform/ps1/ps1_captions.c
    # SPI driver — direct controller polling, lifted from PSn00bSDK
    # examples/io/pads/ (MPL licensed, (C) 2021 spicyjpeg).
    # Replaces the BIOS pad driver, which never auto-polls in our
    # PSn00bSDK 0.24 + DuckStation environment (verified empirically).
    src/platform/ps1/spi.c
    # Memcard persistence for pause-menu settings.
    src/platform/ps1/memcard.c
    # Scene picker — Random / Sequential / Original (Sierra) policies.
    # Net-zero heap; module-level statics only. See scene_picker.c head
    # comment for the malloc-poison guard rationale.
    src/scene/scene_picker.c
    # Holiday table + date-algorithm core (36 holidays, codegen from holidays.yml).
    src/scene/holidays.c
    src/scene/holidays_table.c
    # Walk subsystem — story-loop walks between scenes + freeplay walking.
    # walk.c + walk_data.h came from upstream jc_reborn; calcpath.c is
    # the small Dijkstra over the 6-spot graph. walk_render.c is the
    # shared draw kernel both story-loop and freeplay call. See
    # docs/ps1/walk-implementation-plan.md.
    src/walk/walk.c
    src/walk/calcpath.c
    src/walk/walk_render.c
    src/walk_pilot.c
    src/scene_freeplay/scene_freeplay.c
    # LibC stubs for missing functions
    src/platform/ps1/ps1_stubs.c
    src/platform/ps1/ps1_stdio_stubs.c
    src/platform/ps1/ps1_runtime_stubs.c
    # Memory region allocator — deterministic 3-region static buffer.
    # See docs/ps1/memory-region-allocator-plan.md.
    src/mem_region.c
    src/generated/pack_header_metrics.c
)

# Ownership fragments are textually included by their owning .c files. Keep
# them in the target as HEADER_FILE_ONLY so IDEs show them without compiling
# them as standalone translation units and perturbing symbol/layout order.
set(INCLUDE_FRAGMENTS
    src/ads/restore_pilot.c.inc
    src/ads/replay_telemetry.c.inc
    src/ads/script_index.c.inc
    src/ads/scene_threads.c.inc
    src/ads/interpreter.c.inc
    src/ads/playback_loop.c.inc
    src/ads/island_walk.c.inc
    src/ads/foreground_bridge.c.inc
    src/foreground_pilot/backdrop_public.c.inc
    src/foreground_pilot/scene_catalog.c.inc
    src/foreground_pilot/pack_metadata.c.inc
    src/foreground_pilot/runtime_memory.c.inc
    src/foreground_pilot/backdrop_clean.c.inc
    src/foreground_pilot/stream_runtime.c.inc
    src/graphics_ps1/frame.c.inc
    src/graphics_ps1/loaders.c.inc
    src/graphics_ps1/composite.c.inc
    src/graphics_ps1/draw_primitives.c.inc
    src/graphics_ps1/background_tiles.c.inc
    src/graphics_ps1/clean_rects.c.inc
    src/graphics_ps1/background_screen.c.inc
    src/graphics_ps1/screen_load.c.inc
    src/graphics_ps1/capture_zones.c.inc
    src/pause_menu/lifecycle.c.inc
    src/pause_menu/text_panel.c.inc
    src/pause_menu/presentation.c.inc
    src/pause_menu/option_helpers.c.inc
    src/pause_menu/scene_explorer.c.inc
    src/pause_menu/input.c.inc
    src/pause_menu/update.c.inc
    src/ps1_debug/text.c.inc
    src/ps1_debug/bsod.c.inc
    src/ps1_features/scene_control.c.inc
    src/ps1_features/direct_control.c.inc
    src/ps1_features/clock.c.inc
    src/ps1_features/accessors.c.inc
    src/resource/host_parsers.c.inc
    src/resource/catalog.c.inc
    src/resource/lru.c.inc
    src/scene_freeplay/catalog.c.inc
    src/scene_freeplay/backdrop.c.inc
    src/scene_freeplay/actions.c.inc
    src/scene_freeplay/render.c.inc
    src/scene_freeplay/runtime.c.inc
)

set_source_files_properties(
    ${INCLUDE_FRAGMENTS}
    PROPERTIES HEADER_FILE_ONLY TRUE
)

source_group("PS1\\Debug" FILES
    src/platform/ps1/ps1_debug.c
    src/ps1_debug/text.c.inc
    src/ps1_debug/bsod.c.inc
)
source_group("PS1\\Stubs" FILES
    src/platform/ps1/ps1_stubs.c
    src/platform/ps1/ps1_stdio_stubs.c
    src/platform/ps1/ps1_runtime_stubs.c
)
source_group("Resources" FILES
    src/resource/resource.c
    src/resource/host_parsers.c.inc
    src/resource/catalog.c.inc
    src/resource/lru.c.inc
)
source_group("PS1\\Freeplay" FILES
    src/scene_freeplay/scene_freeplay.c
    src/scene_freeplay/catalog.c.inc
    src/scene_freeplay/backdrop.c.inc
    src/scene_freeplay/actions.c.inc
    src/scene_freeplay/render.c.inc
    src/scene_freeplay/runtime.c.inc
)
source_group("ADS" FILES
    src/ads/ads.c
    src/ads/restore_pilot.c.inc
    src/ads/replay_telemetry.c.inc
    src/ads/script_index.c.inc
    src/ads/scene_threads.c.inc
    src/ads/interpreter.c.inc
    src/ads/playback_loop.c.inc
    src/ads/island_walk.c.inc
    src/ads/foreground_bridge.c.inc
)
source_group("PS1\\ForegroundPilot" FILES
    src/foreground_pilot/foreground_pilot.c
    src/foreground_pilot/backdrop_public.c.inc
    src/foreground_pilot/scene_catalog.c.inc
    src/foreground_pilot/pack_metadata.c.inc
    src/foreground_pilot/runtime_memory.c.inc
    src/foreground_pilot/backdrop_clean.c.inc
    src/foreground_pilot/stream_runtime.c.inc
)
source_group("PS1\\Graphics" FILES
    src/graphics_ps1/graphics_ps1.c
    src/graphics_ps1/frame.c.inc
    src/graphics_ps1/loaders.c.inc
    src/graphics_ps1/composite.c.inc
    src/graphics_ps1/draw_primitives.c.inc
    src/graphics_ps1/background_tiles.c.inc
    src/graphics_ps1/clean_rects.c.inc
    src/graphics_ps1/background_screen.c.inc
    src/graphics_ps1/screen_load.c.inc
    src/graphics_ps1/capture_zones.c.inc
)
source_group("PS1\\Features" FILES
    src/ps1_features/ps1_features.c
    src/ps1_features/scene_control.c.inc
    src/ps1_features/direct_control.c.inc
    src/ps1_features/clock.c.inc
    src/ps1_features/accessors.c.inc
)
source_group("PS1\\PauseMenu" FILES
    src/pause_menu/pause_menu.c
    src/pause_menu/lifecycle.c.inc
    src/pause_menu/text_panel.c.inc
    src/pause_menu/presentation.c.inc
    src/pause_menu/option_helpers.c.inc
    src/pause_menu/scene_explorer.c.inc
    src/pause_menu/input.c.inc
    src/pause_menu/update.c.inc
)

# Size-optimize only translation units that have passed the exact fishing1
# gate. Most hot files stay at the SDK default because code phase is fragile.
set_source_files_properties(
    src/jc_reborn.c
    src/core/utils.c
    src/scene/island.c
    src/platform/ps1/ps1_debug.c
    src/foreground_pilot/foreground_pilot.c
    src/scene/holidays.c
    src/resource/resource.c
    src/platform/ps1/sound_ps1.c
    src/platform/ps1/events_ps1.c
    src/platform/ps1/ps1_pad_script.c
    src/pause_menu/pause_menu.c
    src/platform/ps1/ps1_stubs.c
    src/platform/ps1/ps1_stdio_stubs.c
    src/platform/ps1/ps1_runtime_stubs.c
    src/platform/ps1/ps1_captions.c
    src/platform/ps1/memcard.c
    src/scene_freeplay/scene_freeplay.c
    src/scene/scene_picker.c
    src/mem_region.c
    src/generated/pack_header_metrics.c
    PROPERTIES COMPILE_OPTIONS "-Os"
)

# Create PS1 executable
# Note: GPREL = use GP-relative addressing for faster data access
psn00bsdk_add_executable(jcreborn GPREL ${SOURCES})
target_sources(jcreborn PRIVATE ${INCLUDE_FRAGMENTS})

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ads
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/foreground_pilot
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_ps1
        ${CMAKE_CURRENT_SOURCE_DIR}/src/host
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem_region
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pause_menu
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps1
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ps1_features
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resource
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scene
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scene_freeplay
        ${CMAKE_CURRENT_SOURCE_DIR}/src/walk
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
