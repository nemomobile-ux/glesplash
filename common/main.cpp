#include <cmath>
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <sys/time.h>

#include <linux/omapfb.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "glesplash.h"

GLfloat
   norm_x    =  0.0,
   norm_y    =  0.0,
   offset_x  =  0.0,
   offset_y  =  0.0,
   p1_pos_x  =  0.0,
   p1_pos_y  =  0.0;

bool        update_pos = false;

#define WIDTH 854
#define HEIGHT 480


void signal_callback_handler(int sig)
{
    cerr << "Cleaning up" << endl;
    eglDestroyContext(egl_display, egl_context);
    eglDestroySurface(egl_display, egl_surface);
    eglTerminate(egl_display);

    exit(sig);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        cerr << "You must specify a filename to load" << endl;
        return EXIT_FAILURE;
    }

    cerr << "Creating EGL context" << endl;
    if(create_egl_context() == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    cerr << "Initializing EGL context" << endl;
    if(init_gl_context() == EXIT_FAILURE)
    {
        eglDestroyContext(egl_display, egl_context);
        eglDestroySurface(egl_display, egl_surface);
        eglTerminate(egl_display);
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

    int fd = open("/dev/fb0", O_RDWR);
    struct omapfb_update_window update;
    while (true) {    // the main loop
        render();

        update.x = 0;
        update.y = 0;
        update.width = WIDTH;
        update.height = HEIGHT;
        update.format = OMAPFB_COLOR_RGB565;
        update.out_x = 0;
        update.out_y = 0;
        update.out_width = WIDTH;
        update.out_height = HEIGHT;
        if (ioctl(fd, OMAPFB_UPDATE_WINDOW, &update) < 0) {
            perror("Could not ioctl(OMAPFB_UPDATE_WINDOW)");
        }
        break;

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
    return EXIT_SUCCESS;
}
