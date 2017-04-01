QT       += core gui widgets

TARGET = nisview
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        MainWindow.cpp \
    FileChunk.cpp \
    FormatSpec.cpp \
    formats/ArchiveNIS.cpp \
    FileChunkModel.cpp

HEADERS  += MainWindow.h \
    FileChunk.h \
    FormatSpec.h \
    formats/ArchiveNIS.h \
    FileChunkModel.h

FORMS    += MainWindow.ui
