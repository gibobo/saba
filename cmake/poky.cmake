#include(${CMAKE_CURRENT_LIST_DIR}/prosense.cmake)
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
endif()
macro(set_internal_variable varName varDesc)
    if("${${varName}}" STREQUAL "")
        if("$ENV{${varName}}" STREQUAL "")
            message(FATAL_ERROR
                "Please define environment variable ${Red}${varName}${ColourReset} (${Yellow}${varDesc}${ColourReset})")
        endif()
        set(${varName} $ENV{${varName}} CACHE INTERNAL ${varDesc})
    endif()
endmacro()

macro(import_toolchain_config)
    get_property(IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE)
    set(TOOLCHAIN_CONFIG_FILE "toolchain.config.cmake")
    if(IN_TRY_COMPILE)
        # inherit settings in recursive loads
        include("${CMAKE_CURRENT_SOURCE_DIR}/../../${TOOLCHAIN_CONFIG_FILE}"
            OPTIONAL)
    endif()
endmacro()

macro(export_toolchain_config)
    if(NOT IN_TRY_COMPILE)
        # export toolchain settings for the try_compile() command
        set(__toolchain_config "")
        foreach(__var ${ARGN})
            if(DEFINED ${__var})
                if(${__var} MATCHES " ")
                    set(__set_var
                        "set(${__var} \"${${__var}}\" CACHE INTERNAL \"\")")
                else()
                    set(__set_var
                        "set(${__var} ${${__var}} CACHE INTERNAL \"\")")
                endif()
                set(__toolchain_config
                    "${__toolchain_config}${__set_var}\n")
            endif()
        endforeach()
        file(WRITE "${CMAKE_BINARY_DIR}/${TOOLCHAIN_CONFIG_FILE}"
            "${__toolchain_config}")
        unset(__toolchain_config)
    endif()
endmacro()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

if(CMAKE_TOOLCHAIN_FILE)
    # touch toolchain variable to suppress "unused variable" warning
endif()

set(CMAKE_SIZEOF_VOID_P 32)

# Neet to enable, otherwise always update binaries during installation
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    "${TARGET_ROOTFS_DIR}/lib"
    "${TARGET_ROOTFS_DIR}/usr/lib"
    "${TARGET_ROOTFS_DIR}/usr/local/lib")

set(CMAKE_FIND_ROOT_PATH "${TARGET_ROOTFS_DIR}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

include_directories(SYSTEM "${TARGET_ROOTFS_DIR}/usr/include")

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(ELINUX TRUE)
add_definitions(-DLINUX)
add_definitions(-DIMX6)

#-------------------------------------------------
if(CMAKE_VERSION VERSION_LESS 3.6)
    include(CMakeForceCompiler)
    CMAKE_FORCE_C_COMPILER(arm-poky-gnueabi-gcc GNU)
    CMAKE_FORCE_CXX_COMPILER(arm-poky-gnueabi-g++ GNU)
endif()

import_toolchain_config()

set_internal_variable(POKY_TOOLCHAIN_DIR 
    "poky toolchain directory, ex:/opt/fsl-imx-fb/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux")
set_internal_variable(TARGET_ROOTFS_DIR 
    "Target root filesystem, ex: /opt/fsl-imx-fb/4.1.15-2.1.0/sysroots/cortexa9hf-neon-poky-linux-gnueabi")
# below settings is from
#    /opt/fsl-imx-fb/4.1.15-2.1.0/environment-setup-cortexa9hf-neon-poky-linux-gnueabi
set(TARGET_TOOLCHAIN_PREFIX
    "${POKY_TOOLCHAIN_DIR}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-")
set(CMAKE_C_COMPILER "${TARGET_TOOLCHAIN_PREFIX}gcc")
set(CMAKE_C_COMPILER_AR "${TARGET_TOOLCHAIN_PREFIX}ar")
set(CMAKE_C_COMPILER_RANLIB "${TARGET_TOOLCHAIN_PREFIX}ranlib")
set(CMAKE_CXX_COMPILER "${TARGET_TOOLCHAIN_PREFIX}g++")
set(CMAKE_CXX_COMPILER_AR "${TARGET_TOOLCHAIN_PREFIX}ar")
set(CMAKE_CXX_COMPILER_RANLIB "${TARGET_TOOLCHAIN_PREFIX}ranlib")
set(POKY_FLAGS
    "-march=armv7-a"
    "-mcpu=cortex-a9"
    "-mfpu=neon"
    "-mfloat-abi=hard"
    CACHE STRING "poky related flags")
string(REPLACE ";" " " POKY_FLAGS "${POKY_FLAGS}")
set(TARGET_LINKER_FLAGS "-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
    CACHE STRING "poky specific linker flags")

set(ENV{PKG_CONFIG_PATH} "${TARGET_ROOTFS_DIR}/usr/lib/pkgconfig")
set(CMAKE_C_FLAGS "${POKY_FLAGS} --sysroot=${TARGET_ROOTFS_DIR} -O2 -pipe -g" CACHE STRING "C++ flags")
set(CMAKE_CXX_FLAGS "${POKY_FLAGS} --sysroot=${TARGET_ROOTFS_DIR} -O2 -pipe -g" CACHE STRING "C++ flags")

set(CMAKE_EXE_LINKER_FLAGS      "${TARGET_LINKER_FLAGS}"
    CACHE STRING "Executable linker flags")
set(CMAKE_SHARED_LINKER_FLAGS   "${TARGET_LINKER_FLAGS}"
    CACHE STRING "Shared linker flags")
set(CMAKE_MODULE_LINKER_FLAGS   "${TARGET_LINKER_FLAGS}"
    CACHE STRING "Module linker flags")

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

export_toolchain_config(
    POKY_TOOLCHAIN_DIR
    TARGET_ROOTFS_DIR
    TARGET_LINKER_FLAGS
)