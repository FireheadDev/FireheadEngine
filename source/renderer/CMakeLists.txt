set(MODULE_NAME Renderer)
set(RENDERER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source/renderer")

file(
    GLOB_RECURSE RENDERER_SRC CONFIGURE_DEPENDS
    ./*.h
    ./*.cpp

    ../../shaders/*.vert
    ../../shaders/*.tesc
    ../../shaders/*.tese
    ../../shaders/*.geom
    ../../shaders/*.frag
    ../../shaders/*.comp

    ../../shaders/*.spv
)

file(
    GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ./*.h
    ./*.cpp
)

file(
	GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    ../../shaders/*.vert
    ../../shaders/*.tesc
    ../../shaders/*.tese
    ../../shaders/*.geom
    ../../shaders/*.frag
    ../../shaders/*.comp
)

# For peace of mind to check that they exist
file(
    GLOB_RECURSE SHADER_COMPILED_FILES CONFIGURE_DEPENDS
    ../../shaders/*.spv
)

set_source_files_properties(${SHADER_FILES} PROPERTIES
        VS_SHADER_TYPE Vertex # not all are vertex shaders, but that's ok here
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" # stop Visual Studio from trying to compile, we'll do it
)

add_compile_definitions(RENDERER_DLL)
add_library(${MODULE_NAME} SHARED ${RENDERER_SRC})
source_group("source" FILES ${SRC_FILES})
source_group("shaders" FILES ${SHADER_FILES})
source_group("shaders/compiled" FILES ${SHADER_COMPILED_FILES})
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${MODULE_NAME} 
    Logger 
    Input
    Vulkan::Vulkan 
    glfw
    ktx
    tinyobjloader
)
