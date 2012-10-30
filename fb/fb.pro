TARGET   = glesplash-fb
TEMPLATE = app
CONFIG  += link_pkgconfig
CONFIG  -= qt

PKGCONFIG += libpng egl glesv2

SOURCES = ../common/main.cpp \
          ../common/png.cpp \
          ../common/shaders.cpp \
          ../common/gl.cpp

HEADERS = ../common/glesplash.h \
          ../common/shaders.h
INCLUDEPATH += ../common

DEFINES += FB

pvrcfg.files = glesplash-fb.ini
pvrcfg.path = /etc/powervr.d/

service.files = glesplash-fb.service
service.path = /lib/systemd/system/

target.path = /usr/bin
INSTALLS += target pvrcfg service
