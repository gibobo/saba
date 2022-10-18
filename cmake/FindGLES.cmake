include(FindPackageHandleStandardArgs)

if(WIN32)
    find_package(Qt5 COMPONENTS Gui REQUIRED)
    find_path(GLES_INCLUDE_DIR "GLES2/gl2.h"
        PATHS ${Qt5Core_INCLUDE_DIRS}
        PATH_SUFFIXES "QtANGLE")
    add_library(GLESv2 STATIC IMPORTED)
    find_library(GLES_LIBRARY_DEBUG libGLESv2d
        PATHS ${Qt5Core_INCLUDE_DIRS}
        PATH_SUFFIXES "../lib")
    find_library(GLES_LIBRARY_RELEASE libGLESv2
        PATHS ${Qt5Core_INCLUDE_DIRS}
        PATH_SUFFIXES "../lib")
    set_target_properties(GLESv2 PROPERTIES
        IMPORTED_LOCATION_DEBUG "${GLES_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${GLES_LIBRARY_RELEASE}"
        INTERFACE_COMPILE_DEFINITIONS "GL_GLEXT_PROTOTYPES"
    )
    set(GLES_LIBRARY GLESv2)

    find_package_handle_standard_args(GLES
        REQUIRED_VARS
            GLES_INCLUDE_DIR
            GLES_LIBRARY_DEBUG
            GLES_LIBRARY_RELEASE)
elseif(UNIX)
    find_path(GLES_INCLUDE_DIR "GLES2/gl2.h")
    find_library(GLES_LIBRARY GLESv2)
    find_package_handle_standard_args(GLES
        REQUIRED_VARS
            GLES_LIBRARY
            GLES_INCLUDE_DIR)
endif()
