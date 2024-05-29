# add support for ktx texture loading
#include_directories(${CMAKE_SOURCE_DIR}/ktx/include)

#target_include_directories(Renderer PUBLIC $ENV{VULKAN_SDK}/Include/)
#target_link_directories(Renderer PUBLIC $ENV{VULKAN_SDK}/Lib/)
# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
#target_compile_options(Renderer PRIVATE "/MD")

#find_library(KTX_LIB_D NAMES ktx_x64_d PATHS ${CMAKE_SOURCE_DIR}/ktx/lib)
#find_library(KTX_LIB_R NAMES ktx_x64_r PATHS ${CMAKE_SOURCE_DIR}/ktx/lib)
#target_link_libraries(Renderer debug ${KTX_LIB_D} optimized ${KTX_LIB_R})

# Should get moved up with rendering files if possible when those are added
file(
	GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
	./shaders/*.hlsl
	./shaders/*.glsl
)
set_source_files_properties( ${SHADER_FILES} PROPERTIES
        VS_SHADER_TYPE Vertex # not all are vertex shaders, but that's ok here
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" # stop VS from compiling, we will do it
)