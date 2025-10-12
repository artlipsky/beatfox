// GLAD OpenGL Loader
// This is a minimal GLAD implementation
// For a full implementation, download from: https://glad.dav1d.de/

#include <glad/glad.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static HMODULE libgl;

static void open_gl(void) {
    libgl = LoadLibraryA("opengl32.dll");
}

static void close_gl(void) {
    FreeLibrary(libgl);
}

static void *get_proc(const char *namez) {
    void *p = (void *)wglGetProcAddress(namez);
    if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) ) {
        p = (void *)GetProcAddress(libgl, namez);
    }
    return p;
}
#else
#include <dlfcn.h>

static void *libgl;

#ifndef __APPLE__
typedef void *(*GLADloadproc)(const char *name);
#else
#include <mach-o/dyld.h>
#endif

static void open_gl(void) {
#ifdef __APPLE__
    libgl = NULL;
#else
    libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!libgl)
        libgl = dlopen("libGL.so", RTLD_LAZY | RTLD_LOCAL);
#endif
}

static void close_gl(void) {
#ifndef __APPLE__
    if (libgl)
        dlclose(libgl);
#endif
}

static void *get_proc(const char *namez) {
#ifdef __APPLE__
    // On macOS, OpenGL functions are directly available
    static void *image = NULL;
    if (!image) {
        image = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
    }
    return image ? dlsym(image, namez) : NULL;
#else
    void *result = NULL;
    if (libgl) {
        result = dlsym(libgl, namez);
    }
    return result;
#endif
}
#endif

// OpenGL function pointers
PFNGLCREATESHADERPROC glad_glCreateShader;
PFNGLSHADERSOURCEPROC glad_glShaderSource;
PFNGLCOMPILESHADERPROC glad_glCompileShader;
PFNGLGETSHADERIVPROC glad_glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
PFNGLDELETESHADERPROC glad_glDeleteShader;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
PFNGLATTACHSHADERPROC glad_glAttachShader;
PFNGLLINKPROGRAMPROC glad_glLinkProgram;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glad_glUseProgram;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
PFNGLGENBUFFERSPROC glad_glGenBuffers;
PFNGLBINDBUFFERPROC glad_glBindBuffer;
PFNGLBUFFERDATAPROC glad_glBufferData;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;

int gladLoadGLLoader(GLADloadproc load) {
    open_gl();

    glad_glCreateShader = (PFNGLCREATESHADERPROC)get_proc("glCreateShader");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC)get_proc("glShaderSource");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC)get_proc("glCompileShader");
    glad_glGetShaderiv = (PFNGLGETSHADERIVPROC)get_proc("glGetShaderiv");
    glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)get_proc("glGetShaderInfoLog");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC)get_proc("glDeleteShader");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)get_proc("glCreateProgram");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC)get_proc("glAttachShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)get_proc("glLinkProgram");
    glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)get_proc("glGetProgramiv");
    glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)get_proc("glGetProgramInfoLog");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC)get_proc("glUseProgram");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)get_proc("glDeleteProgram");
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)get_proc("glGenVertexArrays");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)get_proc("glBindVertexArray");
    glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)get_proc("glDeleteVertexArrays");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC)get_proc("glGenBuffers");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC)get_proc("glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC)get_proc("glBufferData");
    glad_glBufferSubData = (PFNGLBUFFERSUBDATAPROC)get_proc("glBufferSubData");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)get_proc("glDeleteBuffers");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)get_proc("glVertexAttribPointer");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)get_proc("glEnableVertexAttribArray");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)get_proc("glGetUniformLocation");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)get_proc("glUniformMatrix4fv");

    return 1;
}
