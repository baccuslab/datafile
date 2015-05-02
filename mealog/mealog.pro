######################################################################
# Automatically generated by qmake (3.0) Thu Apr 16 21:04:08 2015
######################################################################

TEMPLATE = app
TARGET = mealog
OBJECTS_DIR = build
MOC_DIR = build
QT += network printsupport widgets concurrent
QT_CONFIG -= no-pkg-config
CONFIG += c++11 debug_and_release link_pkgconfig
PKGCONFIG += protobuf
INCLUDEPATH += . include ../ \
			/usr/local/include
LIBS += -L../h5recording/lib -lh5recording \
		-L../messaging/lib -lmessaging \
		-L ../daqclient/lib -ldaqclient
QMAKE_CXXFLAGS += -std=c++11
QMAKE_RPATHDIR += ../h5recording/lib ../messaging/lib ../daqclient/lib

# Input
HEADERS += include/mealogwindow.h \
			include/plotwindow.h \
			include/qcustomplot.h \
			include/settings.h \
			include/windows.h \
			include/channelplot.h
SOURCES += src/main.cpp \
			src/mealogwindow.cpp \
			src/plotwindow.cpp \
			src/qcustomplot.cpp \
			src/settings.cpp \
			src/windows.cpp \
			src/channelplot.cpp
