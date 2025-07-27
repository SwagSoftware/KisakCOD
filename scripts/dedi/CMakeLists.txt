# Build Guard
#if (NOT DEFINED DEDICATED) # This should be renamed to KISAK_DEDICATED
#    return()
#else()
    message("----- KisakCOD Dedicated(MP) -----")
#endif()

project(KisakCOD-dedi)

# Include all common files between mp/sp/dedi
include(${SCRIPTS_DIR}/common_files.cmake)

# Include all files specific to mp
include(${SCRIPTS_DIR}/mp/mp_files.cmake)
include(dedi_files.cmake)

# Create the source groups
include(${SCRIPTS_DIR}/source_groups.cmake)

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

target_compile_definitions(${PROJECT_NAME} PUBLIC KISAK_MP KISAK_DEDICATED DEDICATED)

# We need all of the pre/post-build commands
include(${SCRIPTS_DIR}/pre_build.cmake)
include(${SCRIPTS_DIR}/post_build.cmake)

# Tracey
include(${SCRIPTS_DIR}/extern/tracy.cmake)