######################################################################
# Automatically generated by qmake (3.0) Fri Apr 17 07:52:30 2015
######################################################################

TEMPLATE = subdirs
TARGET = mearec
INCLUDEPATH += . \
               messaging \
               h5recording/include \
               mealog/include \
               tools/include
SUBDIRS += h5recording \
			messaging \
			tools \
			dataclient \
			mealog
QT_CONFIG -= no-pkg-config
CONFIG += ordered c++11 debug_and_release link_pkgconfig
QMAKE_CFLAGS += -Wall -Wextra
QMAKE_CFLAGS_DEBUG += -g -O0
QMAKE_CFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -Wno-deprecated-register
QMAKE_CXXFLAGS_DEBUG += -g -O0
QMAKE_CXXFLAGS_RELEASE += -O3

# Input
HEADERS += messaging/logserver.pb.h \
           dataclient/include/dataclient.h \
		   dataclient/include/mcsclient.h \
           h5recording/include/h5recording.h \
           mealog/include/mealogwindow.h \
           mealog/include/plotwindow.h \
           mealog/include/qcustomplot.h \
           mealog/include/settings.h \
           tools/common/bintools.h \
           tools/common/hdftools.h

SOURCES += messaging/logserver.pb.cc \
		   dataclient/src/dataclient.cc \
           dataclient/src/mcsclient.cc \
           h5recording/src/h5recording.cc \
           mealog/src/main.cc \
           mealog/src/mealogwindow.cc \
           mealog/src/plotwindow.cc \
           mealog/src/qcustomplot.cc \
           mealog/src/settings.cc \
           tools/bin2hdf/bin2hdf.c \
           tools/common/bintools.c \
           tools/hdf2bin/hdf2bin.c \
           tools/common/hdftools.c

