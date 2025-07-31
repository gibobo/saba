cmake_minimum_required(VERSION 3.10...3.27)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(saba LANGUAGES C CXX)

# Check if CMake configure needs to be rerun
if(DEFINED _NEED_RECONFIGURE AND _NEED_RECONFIGURE)
  message(WARNING "CMake configuration needs to be rerun due to GLFW auto-detection!")
  message(STATUS "To fix this, please run cmake configure again.")
  # Clear the flag to avoid repeated warnings
  unset(_NEED_RECONFIGURE CACHE)
endif()

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (SABA_BULLET_ROOT "" CACHE PATH "Bullet Root Directory")
option (SABA_ENABLE_TEST "Enable Google test." on)
option (SABA_ENABLE_GL_TEST "OpenGL test." off)
option (SABA_USE_GLSLANG "glsl Preprocessor : glslang lib" off)
option (SABA_INSTALL "Saba install." off)
set (SABA_GLFW_ROOT "" CACHE PATH "GLFW Root Directory")
option (SABA_FORCE_GLFW_BUILD "Force glfw build." off)
option (SABA_ENABLE_EXAMPLE_VULKAN "Build vulakn's example." off)

set (BULLET_ROOT ${SABA_BULLET_ROOT})

find_package (Bullet)

if (NOT CMAKE_TOOLCHAIN_FILE)
    find_package (OpenGL REQUIRED)
endif ()

if (NOT BULLET_FOUND)
    if(MSVC)
        set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "Use MSVC Runtime Library DLL (/MD or /MDd)" FORCE)
    endif ()
    set(INSTALL_LIBS OFF CACHE BOOL "Set when you want to install libraries" FORCE)
    set(INSTALL_CMAKE_FILES OFF CACHE BOOL "Install generated CMake files" FORCE)
    set(BUILD_CPU_DEMOS OFF CACHE BOOL "Build original Bullet CPU examples" FORCE)
    set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "Set when you want to build the OpenGL3+ demos" FORCE)
    set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Set when you want to build the Bullet 2 demos" FORCE)
    set(BUILD_UNIT_TESTS OFF CACHE BOOL "Build Unit Tests" FORCE)
    set(BUILD_EXTRAS OFF CACHE BOOL "Set when you want to build the extras" FORCE)
    add_subdirectory(external/bullet3)
    set(BULLET_LIBRARIES "Bullet3Common;BulletDynamics;BulletSoftBody;BulletCollision;BulletInverseDynamics;LinearMath")
    set(BULLET_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/bullet3/src)
endif ()

# Use bundled GLFW to solve auto-detection problems
message(STATUS "Using bundled GLFW library")

# If the glfw subdirectory exists, use it directly
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
  
  # Add GLFW subdirectory
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
  
  # Set GLFW-related variables
  set(GLFW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include CACHE PATH "GLFW include directory")
  set(GLFW_LIBRARIES glfw CACHE STRING "GLFW libraries")
  set(GLFW_FOUND TRUE CACHE BOOL "GLFW found")
  
  message(STATUS "Bundled GLFW will be built with this project")
else()
  message(FATAL_ERROR "Could not find GLFW in ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
endif()

# Ensure GLFW headers are included
include_directories(${GLFW_INCLUDE_DIR})

if (SABA_ENABLE_TEST)
    enable_testing()
endif()

if (SABA_ENABLE_EXAMPLE_VULKAN)
    find_package(Vulkan REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/viewer)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/external/gl3w/include)
include_directories(${PROJECT_SOURCE_DIR}/external/gli/include)
include_directories(${PROJECT_SOURCE_DIR}/external/glm/include)
include_directories(${PROJECT_SOURCE_DIR}/external/stb/include)
include_directories(${PROJECT_SOURCE_DIR}/external/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/external/tinyobjloader/include)
include_directories(${PROJECT_SOURCE_DIR}/external/imgui/include)
include_directories(${PROJECT_SOURCE_DIR}/external/json/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/external/lua)
include_directories(${PROJECT_SOURCE_DIR}/external/sol2/include)
include_directories(${PROJECT_SOURCE_DIR}/external/tinyxfileloader/include)
include_directories(${PROJECT_SOURCE_DIR}/external/tinyddsloader/include)
include_directories(${BULLET_INCLUDE_DIRS})

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE)
    ADD_DEFINITIONS(-D_UNICODE)
endif(WIN32)

if(MSVC)
    ADD_DEFINITIONS(/MP)
endif()

add_subdirectory(external)

add_subdirectory(src)

# Only build window application when GLFW is available
if (GLFW_FOUND)
    message(STATUS "GLFW is available - building viewer components")
    
    # Add viewer and test subdirectories
    add_subdirectory(viewer)
    add_subdirectory(gtests)

    # Build main window application
    add_executable(saba_viewer saba_viewer.cpp)
    set (saba_viewer_LIBRARIES SabaViewer)
    
    # On UNIX platforms, add dynamic link libraries
    if (UNIX)
        list (APPEND saba_viewer_LIBRARIES ${CMAKE_DL_LIBS})
    endif ()
    
    # Link libraries to window application
    target_link_libraries(saba_viewer ${saba_viewer_LIBRARIES})

    # Copy resource files to build directory
    add_custom_command(TARGET saba_viewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/viewer/Saba/Viewer/resource
        ${CMAKE_CURRENT_BINARY_DIR}/resource
    )
else()
    message(WARNING "GLFW not found and not building - viewer components will be disabled")
endif()

add_executable(mmd2obj mmd2obj.cpp)
target_link_libraries(mmd2obj Saba)

add_subdirectory(example)

# Install
if (SABA_INSTALL)
    install (DIRECTORY src/Saba DESTINATION include  FILES_MATCHING PATTERN "*.h")
    install (TARGETS saba_viewer RUNTIME DESTINATION bin)
    install (DIRECTORY viewer/Saba/Viewer/resource DESTINATION bin)
    install (TARGETS mmd2obj RUNTIME DESTINATION bin)
endif()
