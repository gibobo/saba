#include "./resource_opengl/saba_gles2.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#if _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif // _WIN32

#include <iostream>
#include <fstream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Saba/Base/Path.h>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMXModel.h>

struct Input
{
    std::string m_modelPath;
    std::vector<std::string> m_vmdPaths;
};

void Usage()
{
    std::cout << "app [-model <pmd|pmx file path>] [-vmd <vmd file path>]\n";
    std::cout << "e.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n";
}

bool SampleMain(std::vector<std::string> &args)
{
    saba_gles2 mmd;
    if (!mmd.Setup(args))
    {
        Usage();
        return false;
    }

    // Initialize glfw
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, mmd.appContext.m_msaaSamples);
#if defined(GLFW_TRANSPARENT_FRAMEBUFFER)
    if (mmd.enableTransparentWindow)
    {
        glfwWindowHint(GLFW_SAMPLES, 0);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    }
#endif // defined(GLFW_TRANSPARENT_FRAMEBUFFER)

    auto window = glfwCreateWindow(1280, 800, "simple mmd viewer", nullptr, nullptr);
    if (window == nullptr)
    {
        return false;
    }

#if _WIN32 && (GLFW_VERSION_MAJOR >= 3) && (GLFW_VERSION_MINOR >= 3) && (GLFW_VERSION_REVISION >= 3)
    // The color key was removed from glfw3.3.3. (Windows)
    if (mmd.enableTransparentWindow)
    {
        HWND hwnd = glfwGetWin32Window(window);
        LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED;
        SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 255, LWA_COLORKEY);
    }
#endif // _WIN32

    glfwMakeContextCurrent(window);

    if (gl3wInit() != 0)
    {
        return false;
    }

    glfwSwapInterval(0);
    glEnable(GL_MULTISAMPLE);

    double fpsTime = saba::GetTime();
    int fpsFrame = 0;
    double saveTime = saba::GetTime();
    while (!glfwWindowShouldClose(window))
    {
        double time = saba::GetTime();
        double elapsed = time - saveTime;
        if (elapsed > 1.0 / 30.0)
        {
            elapsed = 1.0 / 30.0;
        }
        saveTime = time;
        mmd.appContext.m_elapsed = float(elapsed);
        mmd.appContext.m_animTime += float(elapsed);

        if (mmd.enableTransparentWindow)
        {
            mmd.appContext.SetupTransparentFBO();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        else
        {
            glClearColor(1.0f, 0.8f, 0.75f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        mmd.SetScreenSize(width, height);

        glViewport(0, 0, width, height);

        // Setup camera
        if (mmd.appContext.m_vmdCameraAnim)
        {
            mmd.appContext.m_vmdCameraAnim->Evaluate(mmd.appContext.m_animTime * 30.0f);
            const auto mmdCam = mmd.appContext.m_vmdCameraAnim->GetCamera();
            saba::MMDLookAtCamera lookAtCam(mmdCam);
            mmd.appContext.m_viewMat = glm::lookAt(lookAtCam.m_eye, lookAtCam.m_center, lookAtCam.m_up);
            mmd.appContext.m_projMat = glm::perspectiveFovRH(mmdCam.m_fov, float(width), float(height), 1.0f, 10000.0f);
        }
        else
        {
            mmd.appContext.m_viewMat = glm::lookAt(glm::vec3(0, 10, 50), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));
            mmd.appContext.m_projMat = glm::perspectiveFovRH(glm::radians(30.0f), float(width), float(height), 1.0f, 10000.0f);
        }

        mmd.Draw();

        if (mmd.enableTransparentWindow)
        {
            glDisable(GL_MULTISAMPLE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mmd.appContext.m_transparentFbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mmd.appContext.m_transparentMSAAFbo);
            glDrawBuffer(GL_BACK);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(mmd.appContext.m_copyVAO);
#if _WIN32
            glUseProgram(mmd.appContext.m_copyTransparentWindowShader);
#else // !_WIN32
            glUseProgram(mmd.appContext.m_copyShader);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
#endif
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mmd.appContext.m_transparentFboColorTex);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS
        {
            fpsFrame++;
            double time = saba::GetTime();
            double deltaTime = time - fpsTime;
            if (deltaTime > 1.0)
            {
                double fps = double(fpsFrame) / deltaTime;
                std::cout << fps << " fps\n";
                fpsFrame = 0;
                fpsTime = time;
            }
        }
    }

    mmd.appContext.Clear();

    glfwTerminate();

    return true;
}

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

int main(int argc, char **argv)
{
    // if (argc < 2)
    // {
    //     Usage();
    //     return 1;
    // }

    // std::vector<std::string> args(argc);
    std::vector<std::string> args;
    args.push_back("-model");
    args.push_back("D:/AVM/MMD/雪花ラミィ/雪花ラミィ.pmx");
    args.push_back("-vmd");
    args.push_back("D:/AVM/MMD/Addiction_Tda.vmd");
    args.push_back("-vmd");
    args.push_back("D:/AVM/MMD/Addiction_Camera_ver1.01.vmd");
#if _WIN32
    {
        // WCHAR *cmdline = GetCommandLineW();
        // int wArgc;
        // WCHAR **wArgs = CommandLineToArgvW(cmdline, &wArgc);
        for (int i = 0; i < args.size(); i++)
        {
            args[i] = saba::ToUtf8String(args[i]);
            // args[i] = saba::ToUtf8String(wArgs[i]);
        }
    }
#else // _WIN32
    for (int i = 0; i < args.size(); i++)
    {
        args[i] = argv[i];
    }
#endif

    if (!SampleMain(args))
    {
        std::cout << "Failed to run.\n";
        return 1;
    }

    return 0;
}
