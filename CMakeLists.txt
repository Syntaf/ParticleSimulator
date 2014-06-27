#CMake version
cmake_minimum_required(VERSION 2.8)
project(ParticleSimulator)

#enable Release ALWAYS
set(CMAKE_BUILD_TYPE "Release")

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
	)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/shaders")
include_directories("${PROJECT_SOURCE_DIR}/textures")
include_directories("${GLM_ROOT}")
include_directories("${GLEW_ROOT}/include")
MESSAGE( STATUS "Found GLM in ${GLM_ROOT}")
include_directories("${PROJECT_SOURCE_DIR}/common")

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/textures)
file(COPY shaders/vertexshader.vert shaders/fragmentshader.frag 
	DESTINATION ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders)
file(COPY textures/particle.dds 
	DESTINATION ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/textures)

set(EXECUTABLE_NAME "ParticleSimulator")
add_executable(${EXECUTABLE_NAME} main.cpp)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OPENGL_LIBRARY_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
	add_definitions(${OPENGL_DEFINITIONS})
endif()

#SFML libraries
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system window graphics)
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
endif()

#GLEW libraries
add_library(glew_static STATIC IMPORTED)
set_target_properties(glew_static PROPERTIES
	IMPORTED_LOCATION ${GLEW_ROOT}/lib/Release/x64/glew32.lib)
target_link_libraries(${EXECUTABLE_NAME} glew_static)
MESSAGE( STATUS "Found GLEW in ${GLEW_ROOT}")

#Common files
add_subdirectory(common)
target_link_libraries (${EXECUTABLE_NAME} common)


install (TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# CPack packaging
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${myproject_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${myproject_VERSION_MINOR}")
include(CPack)
