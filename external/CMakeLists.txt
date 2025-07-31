# glslang
if (SABA_USE_GLSLANG)
  if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
    add_subdirectory (glslang)
  else ()
    message (STATUS "glslang was not found.")
  endif ()
endif ()

# fcpp
add_subdirectory (fcpp)

# lua
add_subdirectory (lua)

# gl3w - OpenGL loader generation and build
if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gl3w)
  # Enhanced Python detection: try modern Python3 first, then fallback to legacy
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_EXECUTABLE)
      set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    endif()
  endif()

  # If Python3 not found, try legacy Python detection for compatibility
  if(NOT PYTHON_EXECUTABLE)
    find_package(PythonInterp QUIET)
  endif()

  if(PYTHON_EXECUTABLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gl3w/gl3w_gen.py")
    set(GL3W_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gl3w)
    set(GL3W_GENERATED_FILES
      "${GL3W_BINARY_DIR}/src/gl3w.c"
      "${GL3W_BINARY_DIR}/include/GL/gl3w.h"
      "${GL3W_BINARY_DIR}/include/GL/glcorearb.h"
      "${GL3W_BINARY_DIR}/include/KHR/khrplatform.h"
    )
    
    # Check if GL3W files need to be generated
    set(GL3W_NEEDS_GENERATION FALSE)
    foreach(GL3W_FILE ${GL3W_GENERATED_FILES})
      if(NOT EXISTS ${GL3W_FILE})
        set(GL3W_NEEDS_GENERATION TRUE)
        break()
      endif()
    endforeach()
    
    if(GL3W_NEEDS_GENERATION)
      message(STATUS "Automatically executing gl3w_gen.py to generate OpenGL loader files...")
      message(STATUS "Using Python: ${PYTHON_EXECUTABLE}")
      
      # Ensure output directories exist
      file(MAKE_DIRECTORY "${GL3W_BINARY_DIR}/src")
      file(MAKE_DIRECTORY "${GL3W_BINARY_DIR}/include/GL")
      file(MAKE_DIRECTORY "${GL3W_BINARY_DIR}/include/KHR")
      
      # Execute gl3w_gen.py
      execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/gl3w/gl3w_gen.py"
        WORKING_DIRECTORY "${GL3W_BINARY_DIR}"
        RESULT_VARIABLE GL3W_RESULT
        OUTPUT_VARIABLE GL3W_OUTPUT
        ERROR_VARIABLE GL3W_ERROR
      )
      
      if(GL3W_RESULT EQUAL 0)
        message(STATUS "✓ GL3W files generated successfully")
      else()
        message(WARNING "GL3W generation failed: ${GL3W_ERROR}")
        message(STATUS "Please manually execute:")
        message(STATUS "  python tools/generate_gl3w.py --force")
      endif()
    else()
      message(STATUS "GL3W files already exist, skipping automatic generation")
    endif()
  elseif(NOT PYTHON_EXECUTABLE)
    message(STATUS "Python not found, GL3W files need to be generated manually")
    message(STATUS "Please install Python and re-run cmake, or manually execute:")
    message(STATUS "  python tools/generate_gl3w.py --force")
  endif()
else ()
  message (STATUS "gl3w was not found.")
endif ()