cmake_minimum_required(VERSION 3.22.1)

project(kisakcod_android LANGUAGES CXX)

# NOTE: compiling the decompiled engine sources directly is not possible on
# this device family: the decomp assumes 32-bit pointers (struct sizeof
# asserts, in-place zone deserialization) and modern flagships are
# arm64-only. The zone loader is therefore ported as 64-bit-clean code that
# keeps the 32-bit serialized layout inside the zone blocks
# (kisak_zone_loader_android.cpp); engine_shim/ stays for future selective
# source ports.

add_library(kisakcod_android SHARED
    kisak_android.cpp
    kisak_android_fs_runtime.cpp
    kisak_android_native_renderer.cpp
    kisak_cod4_filesystem_android.cpp
    kisak_cod4_virtual_file_android.cpp
    kisak_engine_filesystem_bridge_android.cpp
    kisak_fastfile_android.cpp
    kisak_iwi_texture_android.cpp
    kisak_iwi_wavelet_android.cpp
    kisak_menu_expression_android.cpp
    kisak_menu_scene_android.cpp
    kisak_menu_state_android.cpp
    kisak_world_scene_android.cpp
    kisak_zone_loader_android.cpp
    kisak_zone_parser_android.cpp
    kisak_zone_rawfile_android.cpp
    kisak_platform_android.cpp
)

target_compile_features(kisakcod_android PRIVATE cxx_std_20)

target_link_options(kisakcod_android PRIVATE
    "-Wl,-z,max-page-size=16384"
)

target_link_libraries(kisakcod_android
    EGL
    GLESv3
    android
    log
    z
)
