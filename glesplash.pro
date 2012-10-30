TARGET   = glesplash
TEMPLATE = app
CONFIG  += link_pkgconfig
CONFIG  -= qt

PKGCONFIG += libpng egl glesv2 x11

SOURCES = main.cpp

target.path = /usr/bin
INSTALLS += target
