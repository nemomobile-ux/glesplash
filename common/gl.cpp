#include "glesplash.h"
#include "shaders.h"

#include <EGL/egl.h>

const float vertexArray[] = {
   -1.0,  1.0,  0.0,
   -1.0, -1.0,  0.0,
    1.0, -1.0,  0.0,
    1.0,  1.0,  0.0,
   -1.0,  1.0,  0.0
};

const float textureArray[] = {
    0.0,  1.0,
    0.0,  0.0,
    1.0,  0.0,
    1.0,  1.0,
    0.0,  1.0
};

EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;

GLint
    position_loc,
    texCoord_loc,
    texSampl_loc;

void render()
{
    static int init = 0;

    if(!init) {
#ifdef X11
        XWindowAttributes gwa;
        XGetWindowAttributes(x_display, win, &gwa);
        glViewport(0, 0, gwa.width, gwa.height);
#else
        glViewport(0, 288, 854, 480);
#endif
        glClearColor(1.0, 0.0, 1.0, 1.0);
        init = 1;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glVertexAttribPointer(position_loc, 3, GL_FLOAT, false, 0, vertexArray);
    glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, false, 0, textureArray);
    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(texCoord_loc);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
    eglSwapBuffers(egl_display, egl_surface);
}

int create_egl_context()
{
    EGLConfig egl_config;
    EGLint num_config;
    EGLint attr[] = {
        EGL_BUFFER_SIZE, 16,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLint ctx_attr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

#ifdef X11
    egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
#else
    egl_display = eglGetDisplay(NULL);
#endif
    if(egl_display == EGL_NO_DISPLAY)
    {
        cerr << "Could not get EGL display" << endl;
        return EXIT_FAILURE;
    }

    if(!eglInitialize(egl_display, NULL, NULL))
    {
        cerr << "Unable to initialize EGL" << endl;
        return EXIT_FAILURE;
    }

    if(!eglChooseConfig(egl_display, attr, &egl_config, 1, &num_config))
    {
        cerr << "Failed to configure EGL context" << endl;
        return EXIT_FAILURE;
    }

    if(num_config < 1)
    {
        cerr << "No EGL configurations found" << endl;
        return EXIT_FAILURE;
    }

#ifdef X11
    egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)win, NULL);
#else
    egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)NULL, NULL);
#endif
    if(egl_surface == EGL_NO_SURFACE)
    {
        cerr << "Unable to create EGL surface: " << eglGetError() << endl;
        return EXIT_FAILURE;
    }

    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, ctx_attr);
    if(egl_context == EGL_NO_CONTEXT)
    {
        cerr << "Unable to create EGL context: " << eglGetError() << endl;
        return EXIT_FAILURE;
    }

    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    return EXIT_SUCCESS;
}

int init_gl_context()
{
    ///////  the openGL part  ///////////////////////////////////////////////////////////////
    GLuint shaderProgram  = glCreateProgram();
    GLuint vertexShader   = load_shader(vertex_src, GL_VERTEX_SHADER);
    GLuint fragmentShader = load_shader(fragment_src, GL_FRAGMENT_SHADER);

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    //// now get the locations (kind of handle) of the shaders variables
    position_loc  = glGetAttribLocation(shaderProgram, "aPosition");
    texCoord_loc  = glGetAttribLocation(shaderProgram, "aTexCoord");
    texSampl_loc  = glGetUniformLocation(shaderProgram, "tex");

    if(position_loc < 0) {
        cerr << "Unable to get position attribute" << endl;
        return EXIT_FAILURE;
    }

    if(texCoord_loc < 0) {
        cerr << "Unable to get texCoord attribute" << endl;
        return EXIT_FAILURE;
    }

    if(texSampl_loc < 0) {
        cerr << "Unable to get texSampl uniform" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
