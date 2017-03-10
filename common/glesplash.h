#ifndef _GLESPLASH_H
#define _GLESPLASH_H

#include <iostream>
using namespace std;

#include <stdlib.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

extern EGLDisplay egl_display;
extern EGLContext egl_context;
extern EGLSurface egl_surface;

extern GLint texSampl_loc;

GLuint png_texture_load(const char * file_name, int * width, int * height);

void render();
int create_egl_context();
int init_gl_context();

#endif
