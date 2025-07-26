include(FetchContent)

##### Tracy #####
option ( TRACY_ENABLE "" ON )
option ( TRACY_ON_DEMAND "" ON )
option ( TRACY_ONLY_LOCALHOST "" ON )
#option ( TRACY_FIBERS "" ON )

FetchContent_Declare (
	tracy
	GIT_REPOSITORY https://github.com/wolfpld/tracy.git
	GIT_TAG v0.12.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable ( tracy )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/_deps/tracy-src/public)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ENABLE>)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ON_DEMAND>)
target_link_libraries(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:TracyClient>) # Enable Profiler in Debug
#################