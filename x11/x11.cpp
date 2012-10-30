#include <string.h>

#include "glesplash.h"

Display    *x_display;
Window      win;

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
