######################################################################
# Automatically generated by qmake (3.0) Thu Apr 16 21:05:39 2015
######################################################################

TEMPLATE = lib
TARGET = messaging
VERSION = 0.1.0

DESTDIR = lib
OBJECTS_DIR = build
QT -= core gui
QT_CONFIG -= no-pkg-config
CONFIG += c++11 debug_and_release link_pkgconfig
PKGCONFIG += protobuf
INCLUDEPATH += .
QMAKE_CXXFLAGS += -std=c++11

mac {
	QMAKE_SONAME_PREFIX += @rpath
}

# Input
HEADERS += logserver.pb.h
SOURCES += logserver.pb.cc
