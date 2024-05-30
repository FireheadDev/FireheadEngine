set(MODULE_NAME Logger)
# GLOBs needs to get changed if any more complicated CMake features get used
file(
	GLOB_RECURSE LOGGER_SRC CONFIGURE_DEPENDS
	./*.h
	./*.cpp
)

add_compile_definitions(LOGGER_DLL)
add_library(${MODULE_NAME} SHARED ${LOGGER_SRC})	
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)