#include "./resource_opengl/saba_gles2.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#if _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif // _WIN32
#include <chrono>

double GetTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

bool SampleMain(std::vector<std::string> &args)
{
    saba_gles2 mmd;
    if (!mmd.Parse(args))
    {
        printf("%s [-model <pmd|pmx file path>] [-vmd <vmd file path>]\ne.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n", args[0].c_str());
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
    glfwWindowHint(GLFW_SAMPLES, mmd.GetMsaaSamples());
#if defined(GLFW_TRANSPARENT_FRAMEBUFFER)
    if (mmd.IsEnableTransparent())
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
    if (mmd.IsEnableTransparent())
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

    // Initialize application
    if (!mmd.Setup())
        return false;

    double fpsTime = GetTime();
    int fpsFrame = 0;
    float saveTime = (float)GetTime();
    while (!glfwWindowShouldClose(window))
    {
        float time = (float)GetTime();
        float elapsed = time - saveTime;
        if (elapsed > 1.0 / 30.0)
            elapsed = 1.0 / 30.0;
        saveTime = time;

        mmd.SetupTransparent();
        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        mmd.SetScreenSize(width, height);
        mmd.Evaluate(elapsed);
        mmd.Draw();
        mmd.UpdateTransparent();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS
        {
            fpsFrame++;
            double time = GetTime();
            double deltaTime = time - fpsTime;
            if (deltaTime > 1.0)
            {
                double fps = double(fpsFrame) / deltaTime;
                printf("%.2f fps\n", fps);
                fpsFrame = 0;
                fpsTime = time;
            }
        }
    }

    glfwTerminate();

    return true;
}

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("%s [-model <pmd|pmx file path>] [-vmd <vmd file path>]\ne.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n", argv[0]);
        return 1;
    }

    std::vector<std::string> args(argc);
#if _WIN32
    {
        WCHAR *cmdline = GetCommandLineW();
        int wArgc;
        WCHAR **wArgs = CommandLineToArgvW(cmdline, &wArgc);
        for (int i = 0; i < argc; i++)
        {
            args[i] = saba::ToUtf8String(wArgs[i]);
        }
    }
#else // _WIN32
    for (int i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }
#endif

    if (!SampleMain(args))
    {
        printf("Failed to run.\n");
        return 1;
    }

    return 0;
}
