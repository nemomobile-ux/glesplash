#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <signal.h>
#include <sys/time.h>

#include "glesplash.h"

Display    *x_display;
Window      win;

GLfloat
   norm_x    =  0.0,
   norm_y    =  0.0,
   offset_x  =  0.0,
   offset_y  =  0.0,
   p1_pos_x  =  0.0,
   p1_pos_y  =  0.0;


bool        update_pos = false;

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
