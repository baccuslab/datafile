######################################################################
# Automatically generated by qmake (3.0) Fri Apr 17 08:25:36 2015
######################################################################

TEMPLATE = app
TARGET = bin2hdf
VERSION = 0.1.0

DESTDIR = ../bin
OBJECTS_DIR = ../build
CONFIG += debug_and_release c99
QT -= gui core
QMAKE_CFLAGS += -std=c99

INCLUDEPATH += ../common
LIBS += -L../lib -lmeatools
QMAKE_RPATHDIR += ../lib
linux {
	INCLUDEPATH += /usr/include/hdf5/serial
	LIBS += -L/usr/lib/x86_64-linux-gnu/ -lhdf5_serial
}
mac {
	INCLUDEPATH += /usr/local/include
	LIBS += -L/usr/local/lib -lhdf5
	CONFIG -= app_bundle
}

# Input
HEADERS += ../common/bintools.h ../common/hdftools.h
SOURCES += bin2hdf.c
