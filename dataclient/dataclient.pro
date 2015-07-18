######################################################################
# Automatically generated by qmake (3.0) Thu Apr 16 17:08:15 2015
######################################################################

TEMPLATE = lib
TARGET = dataclient
VERSION = 0.1.0
DESTDIR = lib
OBJECTS_DIR = build
INCLUDEPATH += . include ../
QT += network
QT -= gui
CONFIG += c++11 debug_and_release

mac {
	QMAKE_SONAME_PREFIX += @rpath
}

# Input
HEADERS += include/mcsclient.h \
			include/hdclient.h \
			include/dataclient.h
SOURCES += src/mcsclient.cc \
			src/hdclient.cc \
			src/dataclient.cc \
			../daqsrv/messages.h \
			../daqsrv/nidaq.h 
