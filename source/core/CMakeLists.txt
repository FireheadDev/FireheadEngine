set(MODULE_NAME Core)
# GLOBs needs to get changed if any more complicated CMake features get used
file(
	GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
	./*.h
	./*.cpp
)

add_compile_definitions(CORE_DLL)

add_library(${MODULE_NAME} SHARED ${CORE_SRC})	

target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${MODULE_NAME} Logger)