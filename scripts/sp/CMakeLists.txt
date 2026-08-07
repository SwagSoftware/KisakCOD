message("----- KisakCOD Client(SP) -----")

project(KisakCOD-sp)

# Include all common files between mp/sp/dedi
include(${SCRIPTS_DIR}/common_files.cmake)

# Include all files specific to mp
include(sp_files.cmake)

# Create the source groups
include(${SCRIPTS_DIR}/source_groups.cmake)

include (${PLATFORM_DIR}/${KISAK_PLATFORM}/platform.cmake)

# Add the new executable
add_executable(${PROJECT_NAME}
    ${SRCROOT}
    ${DYNENTITY} 
    ${EFFECTSCORE} 
    ${AIM_ASSIST} 
    ${BGAME} 
    ${BINKLIB} 
    ${CGAME} 
    ${CGAME_MP} 
    ${CLIENT} 
    ${CLIENT_MP} 
    ${COMMON} 
    ${DATABASE} 
    ${DEVGUI} 
    ${GAME} 
    ${GAME_MP} 
    ${GFX_D3D} 
    ${GROUPVOICE} 
    ${GROUPVOICE_SPEEX} 
    ${MSSLIB} 
    ${ODE} 
    ${PHYSICS} 
    ${PHYSICS_ODE} 
    ${QCOMMON} 
    ${RAGDOLL} 
    ${SCRIPT} 
    ${SERVER} 
    ${SERVER_MP} 
    ${SOUND}
    ${SOUND_BACKEND}
    ${SPEEX}
    ${STRINGED} 
    ${UI} 
    ${UI_MP} 
    ${UNIVERSAL} 
    ${WIN32_SRC} 
    ${XANIM} 
    ${ZLIB} 
    ${STEAM_HEADERS}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC KISAK_SP CINEMA USE_SEPARATE_BLIT_TEXTURE)

# Feature flag to swap the sound backend from Miles/MSS to OpenAL (see SOUND_BACKEND above).
if (KISAK_OPENAL)
    target_compile_definitions(${PROJECT_NAME} PUBLIC KISAK_OPENAL)

    # Generate the dr_wav/dr_mp3 single-header implementations in exactly this one TU (see
    # comment in snd_driver_load_obj.cpp). Must stay scoped to this file only, not target-wide,
    # since dr_wav.h/dr_mp3.h are also included (declarations only) elsewhere in the sound module.
    set_source_files_properties(${SRC_DIR}/sound/snd_driver_load_obj.cpp PROPERTIES
        COMPILE_DEFINITIONS "DR_WAV_IMPLEMENTATION;DR_MP3_IMPLEMENTATION"
    )
endif()

# We need all of the pre/post-build commands
include(${SCRIPTS_DIR}/pre_build.cmake)
include(${SCRIPTS_DIR}/post_build.cmake)

# Tracey
include(${SCRIPTS_DIR}/extern/tracy.cmake)

# OpenAL (sound backend, only needed when KISAK_OPENAL is enabled)
if (KISAK_OPENAL)
    include(${SCRIPTS_DIR}/extern/openal.cmake)
endif()