cmake_minimum_required(VERSION 3.16.3)

# Solution name
project(FireheadEngine VERSION 0.0.1)
set(MODULE_NAME App)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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
source_group("source" FILES ${APP_SRC})

# Handle 3rd-party libraries
# Vulkan
if (DEFINED VULKAN_SDK_PATH)
  set(VULKAN_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
  set(VULKAN_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
  set(VULKAN_FOUND "True")
else()
  find_package(Vulkan REQUIRED)
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using Vulkan lib at: ${VULKAN_LIBRARIES}")
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./libraries/src/glfw)
set_target_properties(glfw PROPERTIES FOLDER "libraries/GLFW")
set_target_properties(uninstall PROPERTIES FOLDER "libraries/GLFW")
set_target_properties(update_mappings PROPERTIES FOLDER "libraries/GLFW")

# GLM
add_subdirectory(./libraries/src/glm)
set_target_properties(glm PROPERTIES FOLDER "libraries")

# KTX
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
add_subdirectory(./libraries/src/ktx)
set_target_properties(astcenc-avx2-static PROPERTIES FOLDER "libraries/KTX")
set_target_properties(ktx PROPERTIES FOLDER "libraries/KTX")
set_target_properties(ktx_read PROPERTIES FOLDER "libraries/KTX")
set_target_properties(ktx_version PROPERTIES FOLDER "libraries/KTX")
set_target_properties(obj_basisu_cbind PROPERTIES FOLDER "libraries/KTX")
set_target_properties(objUtil PROPERTIES FOLDER "libraries/KTX")

# tinyobjloader
add_subdirectory(./libraries/src/tinyobjloader)
set_target_properties(tinyobjloader PROPERTIES FOLDER "libraries")

target_include_directories(${MODULE_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}"
	PUBLIC ./source/core
	PUBLIC ./source/logger
	PUBLIC ./source/renderer
	PUBLIC ./source/input
	${VULKAN_INCLUDE_DIRS}
	PUBLIC ./libraries/src/glfw
	PUBLIC ./libraries/src/glm
	PUBLIC ./libraries/src/ktx/include
	PUBLIC ./libraries/src/tinyobjloader
)

# Handle 1st-party modules
add_subdirectory(./source/core)
add_subdirectory(./source/logger)
add_subdirectory(./source/renderer)
add_subdirectory(./source/input)

target_link_directories(
	${MODULE_NAME}
	PRIVATE ./source/core
	PRIVATE ./source/logger
	PRIVATE ./source/renderer
	PRIVATE ./source/input
	PUBLIC ${VULKAN_LIBRARIES}
	PRIVATE ./libraries/src/glfw
	PRIVATE ./libraries/src/glm
	PRIVATE ./libraries/src/ktx
	PRIVATE ./libraries/src/tinyobjloader
)

target_link_libraries(${MODULE_NAME}
	Core
	Logger
	Renderer
	Input
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)
set_property(TARGET ${MODULE_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_property(TARGET ${MODULE_NAME} PROPERTY DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
