#CMake version
cmake_minimum_required(VERSION 2.8)
project(ParticleSimulator)

set(EXECUTABLE_NAME "ParticleSimulator")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
	)

#option to enable OpenCL experimental
option (USE_OPENCL
	"Use OpenCL experimental implementation" OFF)

#include sources
set(HEADERS
		common/shader.hpp
		common/texture.hpp
		common/controls.hpp
)
#include headers
set(SOURCES
		main.cpp
		common/shader.cpp
		common/texture.cpp
		common/controls.cpp
)

#assure roots are properly set before proceeding
if(GLM_ROOT STREQUAL "")
	message(ERROR "Invalid path provided for GLM_ROOT")
endif()
if(SFML_ROOT STREQUAL "")
	message(ERROR "Invalid path provided for SFML_ROOT")
endif()
if(GLEW_ROOT STREQUAL "")
	message(ERROR "Invalid path provided for GLEW_ROOT")
endif()

#include needed directories
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/shaders")
include_directories("${PROJECT_SOURCE_DIR}/textures")
include_directories("${GLM_ROOT}")
include_directories("${GLEW_ROOT}/include")
include_directories("${PROJECT_SOURCE_DIR}/common")
message(STATUS "GLM included at ${GLM_ROOT}")

#copy shaders and textures into release folder, TEMP solution to ths issue
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/textures)
file(COPY shaders/vertexshader.vert shaders/fragmentshader.frag 
	DESTINATION ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders)
file(COPY textures/particle.dds 
	DESTINATION ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/textures)

if(NOT CMAKE_BUILD_TYPE  AND NOT CMAKE_CONFIGURATION_TYPES)
	message( STATUS "Setting build type to 'Release as none was specified.")
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
	#set possible values of built type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug"
		"MinSizeRel" "RelWithDebInfo")
endif()
add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS})

#OpenCL experimental
if(USE_OPENGL)
	find_package(OpenCL REQUIRED)
	include_directories(${OPENCL_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENCL_LIBRARIES})
endif()

#OpenGL Libraries and definitions
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OPENGL_LIBRARY_DIRS})
target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
add_definitions(${OPENGL_DEFINITIONS})

#SFML libraries
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})

#GLEW libraries
add_library(glew_static STATIC IMPORTED)
set_target_properties(glew_static PROPERTIES
	IMPORTED_LOCATION ${GLEW_ROOT}/lib/Release/x64/glew32.lib)
target_link_libraries(${EXECUTABLE_NAME} glew_static)
message( STATUS "Found GLEW in ${GLEW_ROOT}")

install (TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# CPack packaging
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${myproject_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${myproject_VERSION_MINOR}")
include(CPack)
