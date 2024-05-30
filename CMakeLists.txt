cmake_minimum_required(VERSION 3.16.3)

# Solution name
project(FireheadEngine VERSION 0.0.1)
set(MODULE_NAME App)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# setup target output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# compiled build goes here
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file( 
	GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS
	./source/app/*.h
	./source/app/*.cpp
	./source/app/*.ini
)

add_executable(${MODULE_NAME} ${APP_SRC})

# Handle 3rd-party libraries
# Vulkan
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
  set(Vulkan_FOUND "True")
else()
  find_package(Vulkan REQUIRED)
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./libraries/glfw)
set_target_properties(glfw PROPERTIES FOLDER "Libraries")

# GLM
add_subdirectory(./libraries/glm)
set_target_properties(glm PROPERTIES FOLDER "Libraries")

# KTX
set(KTX_FEATURE_DOC OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_STATIC_LIBRARY OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_JNI OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_PY OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS_CTS OFF CACHE BOOL "" FORCE)

set(KTX_FEATURE_ETC_UNPACK OFF CACHE BOOL "" FORCE)

set(KTX_FEATURE_BASISU_SUPPORT_SSE OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_BASISU_SUPPORT_OPENCL OFF CACHE BOOL "" FORCE)

set(KTX_FEATURE_EMBED_BITCODE OFF CACHE BOOL "" FORCE)

set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "" FORCE)


add_subdirectory(./libraries/ktx)
set_target_properties(ktx PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(astcenc-avx2-static PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(fmt PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(imageio PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktx_read PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktx_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(ktx2check PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktx2check_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(ktx2ktx2 PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktx2ktx2_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(ktxinfo PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktxinfo_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(ktxsc PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktxsc_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(ktxtools PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(ktxtools_version PROPERTIES FOLDER "Libraries/ktx/versions")
set_target_properties(obj_basisu_cbind PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(objUtil PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(toktx PROPERTIES FOLDER "Libraries/ktx")
set_target_properties(toktx_version PROPERTIES FOLDER "Libraries/ktx/versions")


target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
	PUBLIC ./source/core
	PUBLIC ./source/logger
	PUBLIC ./source/renderer
	${Vulkan_INCLUDE_DIRS}
	PUBLIC ./libraries/glfw
	PUBLIC ./libraries/glm
)

# Handle 1st-party modules
add_subdirectory(./source/core)
add_subdirectory(./source/logger)
add_subdirectory(./source/renderer)

target_link_directories(
	${MODULE_NAME}
	PRIVATE ./source/core
	PRIVATE ./source/logger
	PRIVATE ./source/renderer
	PUBLIC ${Vulkan_LIBRARIES}
	PRIVATE ./libraries/glfw
	PRIVATE ./libraries/glm
)

target_link_libraries(${MODULE_NAME}
	Core
	Logger
	Renderer
	Vulkan::Vulkan
	glfw
	glm
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)
set_property(TARGET ${MODULE_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
