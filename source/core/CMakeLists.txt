set(MODULE_NAME Core)
# GLOBs needs to get changed if any more complicated CMake features get used
file(
	GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
	./*.h
	./*.cpp
)
file(
    GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS
    ./*.h
)
file(
    GLOB_RECURSE CORE_IMPLEMENTATION CONFIGURE_DEPENDS
    ./*.cpp
)

add_compile_definitions(CORE_DLL)
add_library(${MODULE_NAME} SHARED ${CORE_SRC})
source_group("Headers" FILES ${CORE_HEADERS})
source_group("Implementations" FILES ${CORE_IMPLEMENTATION})
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)