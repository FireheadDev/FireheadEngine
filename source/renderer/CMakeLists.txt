set(MODULE_NAME Renderer)

file(
	GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
	../../shaders/*.hlsl
	../../shaders/*.glsl
)

set_source_files_properties( ${SHADER_FILES} PROPERTIES
        VS_SHADER_TYPE Vertex # not all are vertex shaders, but that's ok here
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" # stop VS from compiling, we will do it
)

file(
    GLOB_RECURSE RENDERER_SRC CONFIGURE_DEPENDS
    ./*.h
    ./*.cpp
)

add_compile_definitions(RENDERER_DLL)
add_library(${MODULE_NAME} SHARED ${RENDERER_SRC})
target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
)

# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
target_compile_options(Renderer PRIVATE "/MD")

target_link_libraries(${MODULE_NAME} Logger Vulkan::Vulkan)
