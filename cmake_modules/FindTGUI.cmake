#
# Try to find TGUI library and include path.
# Once done this will define
#
# TGUI_FOUND
# TGUI_INCLUDE_DIR
# TGUI_LIBRARY
#

IF (WIN32)
    FIND_PATH( TGUI_INCLUDE_DIR TGUI/TGUI.hpp
        $ENV{TGUI_ROOT}/include
        ${TGUI_ROOT}/include
        ${PROJECT_SOURCE_DIR}/src/nvgl/TGUI/include
        DOC "The directory where TGUI.hpp resides")
    FIND_LIBRARY( TGUI_LIBRARY_RELEASE
        NAMES TGUI
        PATHS
        $ENV{TGUI_ROOT}/build/lib/Release
        ${TGUI_ROOT}/build/lib/Release
        ${PROJECT_SOURCE_DIR}/TGUI/bin
        ${PROJECT_SOURCE_DIR}/TGUI/lib
        DOC "The TGUI release library")
	FIND_LIBRARY( TGUI_LIBRARY_DEBUG
        NAMES TGUI-D
        PATHS
        $ENV{TGUI_ROOT}/build/lib/Debug
        ${TGUI_ROOT}/build/lib/Debug
        ${PROJECT_SOURCE_DIR}/TGUI/bin
        ${PROJECT_SOURCE_DIR}/TGUI/lib
        DOC "The TGUI debug library")
ELSE (WIN32)
    FIND_PATH( TGUI_INCLUDE_DIR TGUI/TGUI.hpp
        ${TGUI_ROOT}/include
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        DOC "The directory where TGUI.hpp resides")
    FIND_LIBRARY( TGUI_LIBRARY_RELEASE
        NAMES tgui
        PATHS
        ${TGUI_ROOT}/lib
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The TGUI release library")
	FIND_LIBRARY( TGUI_LIBRARY_DEBUG
		NAMES tguid
		PATHS
        ${TGUI_ROOT}/lib
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The TGUI debug library")
ENDIF (WIN32)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TGUI_LIBRARY ${TGUI_LIBRARY_DEBUG})
else()
    set(TGUI_LIBRARY ${TGUI_LIBRARY_RELEASE})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(TGUI DEFAULT_MSG TGUI_LIBRARY TGUI_INCLUDE_DIR)

MARK_AS_ADVANCED(TGUI_INCLUDE_DIR TGUI_LIBRARY)
