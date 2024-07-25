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
)

# File in file to reduce redundancy?
file(
	GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    ../../shaders/*.vert
    ../../shaders/*.tesc
    ../../shaders/*.tese
    ../../shaders/*.geom
    ../../shaders/*.frag
    ../../shaders/*.comp
)

set_source_files_properties( ${SHADER_FILES} PROPERTIES
        VS_SHADER_TYPE Vertex # not all are vertex shaders, but that's ok here
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" # stop VS from compiling, we will do it
)

add_compile_definitions(RENDERER_DLL)
add_library(${MODULE_NAME} SHARED ${RENDERER_SRC})
source_group("Shaders" FILES ${SHADER_FILES})
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${MODULE_NAME} 
    Logger 
    Vulkan::Vulkan 
    glfw
    ktx
    tinyobjloader
)
