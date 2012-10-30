TARGET   = glesplash
TEMPLATE = app
CONFIG  += link_pkgconfig
CONFIG  -= qt

PKGCONFIG += libpng egl glesv2 x11

SOURCES = main.cpp \
          ../common/png.cpp \
          ../common/shaders.cpp \
          ../common/gl.cpp

HEADERS = ../common/glesplash.h \
          ../common/shaders.h
INCLUDEPATH += ../common

DEFINES += X11

target.path = /usr/bin
INSTALLS += target
