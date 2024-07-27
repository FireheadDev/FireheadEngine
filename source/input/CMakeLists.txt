set(MODULE_NAME Input)
# GLOBs needs to get changed if any more complicated CMake features get used
file(
	GLOB_RECURSE INPUT_SRC CONFIGURE_DEPENDS
	./*.h
	./*.cpp
)

add_compile_definitions(INPUT_DLL)
add_library(${MODULE_NAME} SHARED ${INPUT_SRC})	
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${MODULE_NAME}
	glfw
)