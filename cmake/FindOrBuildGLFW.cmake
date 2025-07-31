# First try to find system GLFW
if(NOT SABA_FORCE_GLFW_BUILD)
  set(GLFW_ROOT ${SABA_GLFW_ROOT})
  find_package(GLFW QUIET)
endif()

# If system GLFW is not found or user specified to force build GLFW
if(NOT GLFW_FOUND OR SABA_FORCE_GLFW_BUILD)
  message(STATUS "System GLFW not found or forced build requested, using bundled GLFW")
  
  # Ensure GLFW source directory exists
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
    message(FATAL_ERROR "Bundled GLFW source not found at ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
  endif()
  
  # Set GLFW build options
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
  
  # Add bundled GLFW to the build
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
  
  # Set GLFW variables
  set(GLFW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)
  set(GLFW_LIBRARIES glfw)
  set(GLFW_FOUND TRUE)
  set(USING_BUNDLED_GLFW TRUE)
  
  message(STATUS "Bundled GLFW will be built with this project")
else()
  message(STATUS "Using system GLFW found at ${GLFW_LIBRARY}")
  set(USING_BUNDLED_GLFW FALSE)
endif()
