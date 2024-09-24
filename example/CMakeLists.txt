# Example

if (NOT CMAKE_TOOLCHAIN_FILE)
    find_package(Threads)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    # simple_mmd_viewer
    add_executable(simple_mmd_viewer
        simple_mmd_viewer.cpp
        resource_opengl/GL_ShaderProgram.cpp
        resource_opengl/AppContext.cpp
        resource_opengl/Model.cpp
        resource_opengl/saba_gles2.cpp
    )

    # simple_mmd_viewer_glfw
    add_executable(simple_mmd_viewer_glfw
        simple_mmd_viewer_glfw.cpp
    )
    set(simple_mmd_viewer_libs 
        Saba
        gl3w
        glfw
        ${OPENGL_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        )
    message("${simple_mmd_viewer_libs}")
    target_link_libraries(simple_mmd_viewer_glfw 
        PUBLIC
            ${simple_mmd_viewer_libs}
    )
    target_link_libraries(simple_mmd_viewer 
        PUBLIC
            ${simple_mmd_viewer_libs}
    )

    ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_directory
                  ${CMAKE_CURRENT_SOURCE_DIR}/../viewer/Saba/Viewer/resource
                  $<TARGET_FILE_DIR:simple_mmd_viewer>/resource
                  )
else()
    find_package (EGL)
    find_package (GLES)

    # simple_mmd_viewer
    add_executable(mmd
        simple_mmd_viewer_fsl.cpp
        resource_opengl/GL_ShaderProgram.cpp
        resource_opengl/AppContext.cpp
        resource_opengl/Model.cpp
        resource_opengl/saba_gles2.cpp
        resource_fsl/fsl_egl.c
    )

    target_compile_definitions(mmd
        PRIVATE
            -D_GLIBCXX_USE_NANOSLEEP
            -D_GLIBCXX_USE_SCHED_YIELD
            -DEGL_API_FB
            -DGL_GLEXT_PROTOTYPES
    )

    target_link_libraries(mmd
        EGL::EGL
        ${GLES_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        ${BULLET_LIBRARIES}
        Saba
    )

    install(TARGETS mmd DESTINATION home/root)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../viewer/Saba/Viewer/resource DESTINATION home/root)
endif()

# simple_mmd_viewer_dx11
if(WIN32)
    add_executable(simple_mmd_viewer_dx11
        simple_mmd_viewer_dx11.cpp
    )
    target_link_libraries(simple_mmd_viewer_dx11
        D3D11
        Saba
    )
endif()

# simple_mmd_viewer_vulkan
if(SABA_ENABLE_EXAMPLE_VULKAN)
    include_directories(${Vulkan_INCLUDE_DIR})
    add_executable(simple_mmd_viewer_vulkan
        simple_mmd_viewer_vulkan.cpp
    )
    target_link_libraries(simple_mmd_viewer_vulkan
        ${Vulkan_LIBRARY}
        ${simple_mmd_viewer_glfw_LIBRARIES}
        Saba
    )
endif()
