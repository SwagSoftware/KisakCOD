message("----- KisakCOD Client(SP) -----")

project(KisakCOD-sp)

# Include all common files between mp/sp/dedi
include(${SCRIPTS_DIR}/common_files.cmake)

# Include all files specific to mp
include(sp_files.cmake)

# Create the source groups
include(${SCRIPTS_DIR}/source_groups.cmake)

include (${PLATFORM_DIR}/${KISAK_PLATFORM}/platform.cmake)

# Select the sound backend source file: OpenAL when KISAK_SOUND is enabled, otherwise Miles.
if (KISAK_SOUND)
    set(SOUND_BACKEND "${SRC_DIR}/sound/snd_al.cpp")
else()
    set(SOUND_BACKEND "${SRC_DIR}/sound/snd_mss.cpp")
endif()

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
if (KISAK_SOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC KISAK_SOUND)
endif()

# We need all of the pre/post-build commands
include(${SCRIPTS_DIR}/pre_build.cmake)
include(${SCRIPTS_DIR}/post_build.cmake)

# Tracey
include(${SCRIPTS_DIR}/extern/tracy.cmake)

# OpenAL (sound backend, only needed when KISAK_SOUND is enabled)
if (KISAK_SOUND)
    include(${SCRIPTS_DIR}/extern/openal.cmake)
endif()