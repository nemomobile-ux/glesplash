#include <iostream>
using namespace std;

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <png.h>

const char vertex_src[] =
"                                               \
   attribute vec4        aPosition;             \
   attribute vec2        aTexCoord;             \
   varying mediump vec2  tCoord;                \
                                                \
   void main()                                  \
   {                                            \
      gl_Position = aPosition;                  \
      tCoord = aTexCoord;                       \
   }                                            \
";

const char fragment_src[] =
"                                               \
  varying mediump vec2 tCoord;                  \
  uniform sampler2D tex;                        \
                                                \
  void  main()                                  \
  {                                             \
      gl_FragColor = texture2D(tex, tCoord);    \
  }                                             \
";

void print_shader_info_log(GLuint shader)
{
   GLint length;

   glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

   if(length) {
      char* buffer = new char[length];
      glGetShaderInfoLog(shader, length, NULL, buffer);
      cout << "shader info: " <<  buffer << flush;
      delete[] buffer;

      GLint success;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (success != GL_TRUE)   exit(1);
   }
}

GLuint load_shader(const char *shader_source, GLenum type)
{
   GLuint shader = glCreateShader(type);

   glShaderSource(shader, 1, &shader_source, NULL);
   glCompileShader(shader);

   print_shader_info_log(shader);

   return shader;
}

Display    *x_display;
Window      win;

EGLDisplay  egl_display;
EGLContext  egl_context;

GLfloat
   norm_x    =  0.0,
   norm_y    =  0.0,
   offset_x  =  0.0,
   offset_y  =  0.0,
   p1_pos_x  =  0.0,
   p1_pos_y  =  0.0;

GLint
   position_loc,
   texCoord_loc,
   texSampl_loc;

EGLSurface  egl_surface;
bool        update_pos = false;

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

int create_x11_window(int x, int y, int width, int height, const char* title)
{
    x_display = XOpenDisplay(NULL);

    if(!x_display)
    {
        cerr << "cannot connect to X server" << endl;
        return EXIT_FAILURE;
    }

    Window root = DefaultRootWindow(x_display);

    XSetWindowAttributes wa;
    wa.event_mask = ExposureMask; // | PointerMotionMask | KeyPressMask;

    win = XCreateWindow(x_display, root, x, y, width, height, 0,
                        CopyFromParent, InputOutput,
                        CopyFromParent, CWEventMask,
                        &wa);

    wa.event_mask = 0;
    wa.override_redirect = false;
    XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &wa);

    Atom atom;
    atom = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(x_display, win,
                    XInternAtom(x_display, "_NET_WM_STATE", True),
                    XA_ATOM, 32, PropModeReplace, (unsigned char*) &atom, 1);

    XWMHints hints;
    hints.input = True;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    XMapWindow(x_display, win);
    XStoreName(x_display, win, title);

    Atom wm_state = XInternAtom(x_display, "_NET_WM_STATE", False);
    Atom fs_state = XInternAtom(x_display, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));

    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fs_state;
    XSendEvent(x_display, root, False, SubstructureNotifyMask, &xev);

    return EXIT_SUCCESS;
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

    egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
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

    if(num_config != 1)
    {
        cerr << "Multiple configurations returned, got " << num_config << " configurations" << endl;
        return EXIT_FAILURE;
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)win, NULL);
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

void render()
{
    static int init = 0;

    if(!init) {
        XWindowAttributes gwa;
        XGetWindowAttributes(x_display, win, &gwa);
        glViewport(0, 0, gwa.width, gwa.height);
        glClearColor(0.0, 0.0, 0.0, 1.0);
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

// Load Texture PNG
GLuint png_texture_load(const char * file_name, int * width, int * height)
{
    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        return 0;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    cerr << "Image information" << endl;
    cerr << "\tWidth: " << temp_width << endl;
    cerr << "\tHeight: " << temp_height << endl;
    cerr << "\tBits: " << bit_depth << endl;
    cerr << "\tCType: " << color_type << endl;

    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    rowbytes += 3 - ((rowbytes - 1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte* image_data = new png_byte[rowbytes * *height + 15];
    if(!image_data)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep* row_pointers = new png_bytep[*height];
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for(int i = 0; i < *height; i++)
    {
        row_pointers[*height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, temp_width, temp_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    return texture;
}

void signal_callback_handler(int sig)
{
    cerr << "Cleaning up" << endl;
    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);
    XDestroyWindow(x_display, win);
    XCloseDisplay(x_display);
    exit(sig);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        cerr << "You must specify a filename to load" << endl;
        return EXIT_FAILURE;
    }

    cerr << "Creating X11 window" << endl;
    if(!create_x11_window(0, 0, 854, 480, "X11 EGL Splash"))
    {
        return EXIT_FAILURE;
    }

    cerr << "Creating EGL context" << endl;
    if(!create_egl_context())
    {
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return EXIT_FAILURE;
    }

    cerr << "Initializing EGL context" << endl;
    if(!init_gl_context())
    {
        eglDestroyContext(egl_display, egl_context);
        eglDestroySurface(egl_display, egl_surface);
        eglTerminate(egl_display);
        XDestroyWindow(x_display, win);
        XCloseDisplay(x_display);
        return EXIT_FAILURE;
    }

    // Load splash texture
    int iWidth, iHeight;
    GLuint splashTexture = png_texture_load(argv[1], &iWidth, &iHeight);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, splashTexture);

    glUniform1i(texSampl_loc, 0);

    // FPS calculation
    int frames = 0;
    timeval startT, iterT;
    struct timezone tz;

    gettimeofday(&startT, &tz);

    signal(SIGINT, signal_callback_handler);
    signal(SIGHUP, signal_callback_handler);

    // Main Loop
    while (true) {    // the main loop
        render();

        if(++frames % 60 == 0) {
            gettimeofday(&iterT, &tz);
            float dt = iterT.tv_sec - startT.tv_sec + (iterT.tv_usec - startT.tv_usec) * 1e-6;
            cout << "fps: " << frames / dt << endl;
            frames = 0;
            startT = iterT;
        }
    }

    // Clean-Up
    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);
    XDestroyWindow(x_display, win);
    XCloseDisplay(x_display);
    return EXIT_SUCCESS;
}
