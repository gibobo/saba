#include "EGL/egl.h"
#include "./resource_fsl/fsl_egl.h"
#include "./resource_opengl/saba_gles2.h"
#include <signal.h>
#include <assert.h>
#include <chrono>

static EGLDisplay egldisplay;
static EGLConfig eglconfig;
static EGLSurface eglsurface;
static EGLContext eglcontext;
static EGLNativeWindowType eglNativeWindow;
static EGLNativeDisplayType eglNativeDisplayType;
static int screen_width = 0;
static int screen_height = 0;
volatile sig_atomic_t RUN = 1;

double GetTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now.time_since_epoch()).count();
}

void sig_handler(int sig)
{
    printf("\nCaught signal %d, setting flaq to quit.\n", sig);
    RUN = 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("%s [-model <pmd|pmx file path>] [-vmd <vmd file path>]\ne.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n", argv[0]);
        return 1;
    }

    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }

    saba_gles2 mmd;
    if (!mmd.Parse(args))
    {
        printf("%s [-model <pmd|pmx file path>] [-vmd <vmd file path>]\ne.g. app -model model1.pmx -vmd anim1_1.vmd -vmd anim1_2.vmd  -model model2.pmx\n", argv[0]);
        return 1;
    }
    ///////////////////////////////////////////////////////////////////////////////////
    const EGLint s_configAttribs[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_SAMPLES, 2,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE};

    const EGLint ContextAttribList[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE};

    EGLint numconfigs;
    eglNativeDisplayType = fsl_getNativeDisplay();
    egldisplay = eglGetDisplay(eglNativeDisplayType);
    eglInitialize(egldisplay, NULL, NULL);
    assert(eglGetError() == EGL_SUCCESS);
    eglBindAPI(EGL_OPENGL_ES_API);
    eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(numconfigs == 1);
    eglNativeWindow = fsl_createwindow(egldisplay, eglNativeDisplayType);
    assert(eglNativeWindow);
    eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, eglNativeWindow, NULL);
    assert(eglGetError() == EGL_SUCCESS);
    eglcontext = eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT, ContextAttribList);
    assert(eglGetError() == EGL_SUCCESS);
    eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
    assert(eglGetError() == EGL_SUCCESS);

    eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &screen_width);
    eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &screen_height);
    assert(eglGetError() == EGL_SUCCESS);
    assert(screen_width);
    assert(screen_height);
    ///////////////////////////////////////////////////////////////////////////////////
    // Initialize application
    if (!mmd.Setup())
        return 1;

    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glViewport(0, 0, screen_width, screen_height);
    mmd.SetScreenSize(screen_width, screen_height);
    const double elapsed_min = 1.0 / 30.0;
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    unsigned int count = 0;
    double rec_time = GetTime();
    double saveTime = rec_time;
    double now;
    while (RUN)
    {
        now = GetTime();
        double elapsed = now - saveTime;
        double deltaTime = now - rec_time;
        count++;
        if (deltaTime > 10.0)
        {
            double fps = (double)count / deltaTime;
            printf("[FPS] %.2lf\n", fps);
            count = 0;
            rec_time = now;
        }
        saveTime = now;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        mmd.Evaluate(elapsed < elapsed_min ? elapsed : elapsed_min);
        mmd.Draw();

        eglSwapBuffers(egldisplay, eglsurface);
        assert(eglGetError() == EGL_SUCCESS);
    }
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    eglSwapBuffers(egldisplay, eglsurface);
    eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(egldisplay, eglcontext);
    eglDestroySurface(egldisplay, eglsurface);
    fsl_destroywindow(eglNativeWindow, eglNativeDisplayType);
    eglTerminate(egldisplay);
    eglReleaseThread();

    return 0;
}
