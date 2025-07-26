# Build Guard
#if (NOT DEFINED KISAK_MP)
#    return()
#else()
    message("----- KisakCOD Client(MP) -----")
#endif()

project(KisakCOD-mp)

# Include all common files between mp/sp/dedi
include(${SCRIPTS_DIR}/common_files.cmake)

# Include all files specific to mp
include(mp_files.cmake)

# Create the source groups
include(${SCRIPTS_DIR}/source_groups.cmake)

message(${SRC_DIR})

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
    ${SPEEX} 
    ${STRINGED} 
    ${UI} 
    ${UI_MP} 
    ${UNIVERSAL} 
    ${WIN32} 
    ${XANIM} 
    ${ZLIB} 
    ${STEAM_HEADERS}
)

# Add multiplayer flag
target_compile_definitions(${PROJECT_NAME} PUBLIC KISAK_MP)

# We need all of the pre/post-build commands
include(${SCRIPTS_DIR}/pre_build.cmake)
include(${SCRIPTS_DIR}/post_build.cmake)

# Tracey
include(${SCRIPTS_DIR}/extern/tracy.cmake)