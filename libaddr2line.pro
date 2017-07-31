###########################################################################################
#    libaddr2line.pro is part of libaddr2line
#
#    Copyright (C) 2014
#       403fd4d072f534ee5bd7da6efc9462f3995bb456bad644cd9bb7bcaad314b02d source@appudo.com
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, 51 Franklin Street - Fifth Floor, Boston,
#    MA 02110-1301, USA.
#
###########################################################################################

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_PRE_LINK =
QMAKE_POST_LINK =
QMAKE_RPATHDIR =
QMAKE_RPATH =

INCLUDEPATH += /usr/include/c++/v1 \
               /usr/include

QMAKE_CXXFLAGS += -Xclang -dwarf-column-info -c -Wno-conversion-null -fno-rtti -fmessage-length=0 -fvisibility-inlines-hidden -fvisibility=hidden -std=c++11 -stdlib=libc++ -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
QMAKE_CXXFLAGS += -fPIC

LIBS += -lc++ -lbfd

QMAKE_LFLAGS += -shared
# -pie
QMAKE_LFLAGS -= -ccc-gcc-name g++
QMAKE_LFLAGS += -Wl,-z,noexecstack

QMAKE_CXXFLAGS_DEBUG -= -g -g0 -g1 -g2 -O -O1 -O2 -O3
QMAKE_CXXFLAGS_RELEASE -= -g -g0 -g1 -g2 -O -O1 -O2 -O3
QMAKE_CFLAGS_DEBUG -= -g -g0 -g1 -g2 -O -O1 -O2 -O3
QMAKE_CFLAGS_RELEASE -= -g -g0 -g1 -g2 -O -O1 -O2 -O3

QMAKE_CXXFLAGS_DEBUG *= -g3 -O0
QMAKE_CFLAGS_DEBUG *= -g3 -O0

QMAKE_CXXFLAGS_RELEASE *= -g0 -O3 -DNDEBUG
QMAKE_CFLAGS_RELEASE *= -g0 -O3 -DNDEBUG

MACHINE = $$system(uname -m)
CONFIG(release, debug|release) : DESTDIR = $$_PRO_FILE_PWD_/Release.$$MACHINE
CONFIG(debug, debug|release)   : DESTDIR = $$_PRO_FILE_PWD_/Debug.$$MACHINE
CONFIG(force_debug_info)       : DESTDIR = $$_PRO_FILE_PWD_/Profile.$$MACHINE

MAKEFILE = $$DESTDIR/Makefile
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

HEADERS += \
    src/libaddr2line.h
SOURCES += \
    src/libaddr2line.cpp

DISTFILES += \
    binutils_2.25.orig.tar.gz \
    COPYING3
