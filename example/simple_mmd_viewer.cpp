#include "./resource_opengl/saba_gles2.h"
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif // _WIN32
#include <chrono>
#include <thread>
#include <iostream>

double GetTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

// 全域變數來存儲MMD實例
saba_gles2* g_mmd = nullptr;

// 鍵盤回調函數
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && g_mmd != nullptr)
    {
        switch (key)
        {
        case GLFW_KEY_O:
            // 切換遮擋剔除
            g_mmd->SetOcclusionCullingEnabled(!g_mmd->IsOcclusionCullingEnabled());
            printf("Occlusion Culling: %s\n", 
                   g_mmd->IsOcclusionCullingEnabled() ? "Enabled" : "Disabled");
            break;
        case GLFW_KEY_F:
            // 切換視錐剔除
            g_mmd->SetFrustumCullingEnabled(!g_mmd->IsFrustumCullingEnabled());
            printf("Frustum Culling: %s\n", 
                   g_mmd->IsFrustumCullingEnabled() ? "Enabled" : "Disabled");
            break;
        case GLFW_KEY_B:
            // 切換邊界框顯示
            g_mmd->SetBoundingBoxesVisible(!g_mmd->AreBoundingBoxesVisible());
            printf("Bounding Boxes: %s\n", 
                   g_mmd->AreBoundingBoxesVisible() ? "Visible" : "Hidden");
            break;
        case GLFW_KEY_H:
            // 顯示幫助信息
            printf("\n--- Keyboard Controls ---\n");
            printf("O: Toggle Occlusion Culling\n");
            printf("F: Toggle Frustum Culling\n");
            printf("B: Toggle Bounding Box Display\n");
            printf("S: Switch Shader Performance Mode\n");
            printf("H: Show this help\n");
            printf("ESC: Exit application\n");
            printf("------------------------\n\n");
            break;
        case GLFW_KEY_S:
            // 切換著色器效能模式
            g_mmd->SwitchShaderPerformanceMode();
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

bool SampleMain(std::vector<std::string> &args)
{
    saba_gles2 mmd;
    g_mmd = &mmd; // 設置全域指標
    
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

    auto window = glfwCreateWindow(320, 240, "simple mmd viewer", nullptr, nullptr);
    if (window == nullptr)
    {
        return false;
    }

#if defined(_WIN32) && (GLFW_VERSION_MAJOR >= 3) && (GLFW_VERSION_MINOR >= 3) && (GLFW_VERSION_REVISION >= 3)
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

    // 啟用垂直同步以節省GPU資源並提供穩定幀率
    glfwSwapInterval(1);
    glEnable(GL_MULTISAMPLE);

    // 設置鍵盤回調
    glfwSetKeyCallback(window, KeyCallback);

    // Initialize application
    if (!mmd.Setup())
        return false;

    // 顯示控制說明
    printf("MMD Viewer (O: Occlusion Culling, F: Frustum Culling, B: Bounding Boxes, H: Help)\n");

    double fpsTime = GetTime();
    int fpsFrame = 0;
    float saveTime = (float)GetTime();
    const float targetFrameTime = 1.0f / 60.0f; // 目標60FPS
    
    // 緩存視窗大小，減少頻繁查詢
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    int lastWidth = width, lastHeight = height;
    
    while (!glfwWindowShouldClose(window))
    {
        float time = (float)GetTime();
        float elapsed = time - saveTime;
        
        // 幀率控制：只有達到目標幀間隔時才更新
        if (elapsed >= targetFrameTime)
        {
            saveTime = time;
        mmd.SetupTransparent();
        glClearColor(1.0f, 0.8f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // 只在視窗大小改變時更新viewport
        glfwGetFramebufferSize(window, &width, &height);
        if (width != lastWidth || height != lastHeight) {
            glViewport(0, 0, width, height);
            mmd.SetScreenSize(width, height);
            lastWidth = width;
            lastHeight = height;
        }
        mmd.SetScreenSize(width, height);
        mmd.Evaluate(elapsed);
        mmd.Draw();
        mmd.UpdateTransparent();

        glfwSwapBuffers(window);
        
        // FPS統計與遮擋剔除統計
        {
            fpsFrame++;
            double time = GetTime();
            double deltaTime = time - fpsTime;
            if (deltaTime > 1.0)
            {
                double fps = double(fpsFrame) / deltaTime;
                int visibleModels = mmd.GetVisibleModelCount();
                int totalModels = mmd.GetTotalModelCount();
                printf("%.2f fps | Models: %d/%d visible | Culled: %d\n", 
                       fps, visibleModels, totalModels, totalModels - visibleModels);
                fpsFrame = 0;
                fpsTime = time;
            }
        }
        }
        else
        {
            // 如果還沒到更新時間，短暫休眠避免空轉
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        glfwPollEvents();
    }

    // 清理全域指標
    g_mmd = nullptr;
    glfwTerminate();

    return true;
}

#if defined(_WIN32)
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
#if defined(_WIN32)
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
