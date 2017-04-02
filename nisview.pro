QT       += core gui widgets

TARGET = nisview
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        MainWindow.cpp \
    FileChunk.cpp \
    FormatSpec.cpp \
    formats/ArchiveNIS.cpp \
    FileChunkModel.cpp \
    formats/CompressNIS.cpp \
    formats/ImageNIS.cpp

HEADERS  += MainWindow.h \
    FileChunk.h \
    FormatSpec.h \
    formats/ArchiveNIS.h \
    FileChunkModel.h \
    Endianness.h \
    formats/CompressNIS.h \
    formats/ImageNIS.h

FORMS    += MainWindow.ui
