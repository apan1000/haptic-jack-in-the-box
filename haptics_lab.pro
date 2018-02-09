TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += Main.cpp
HEADERS += Assignment.h \
    MusicBox.h

MODE = Debug
#MODE = Release

win32{
    CHAI3D = D:/chai3d-3.2.0

    DEFINES += WIN64
    DEFINES += D_CRT_SECURE_NO_DEPRECATE
    QMAKE_CXXFLAGS += /EHsc /MP

    INCLUDEPATH += $${CHAI3D}/src
    INCLUDEPATH += $${CHAI3D}/external/Eigen
    INCLUDEPATH += $${CHAI3D}/external/glew/include
    INCLUDEPATH += $${CHAI3D}/extras/GLFW/include

    DEPENDPATH += $${CHAI3D}/src
    LIBS += -L$${CHAI3D}/lib/$${MODE}/x64/ -lchai3d
    LIBS += -lglu32 -lopengl32 -lwinmm
    LIBS += -L$${CHAI3D}/extras/GLFW/lib/$${MODE}/x64/ -lglfw
    LIBS += -lglu32 -lOpenGl32 -lglu32 -lOpenGl32 -lwinmm -luser32
    LIBS += kernel32.lib
    LIBS += user32.lib
    LIBS += gdi32.lib
    LIBS += winspool.lib
    LIBS += comdlg32.lib
    LIBS += advapi32.lib
    LIBS += shell32.lib
    LIBS += ole32.lib
    LIBS += oleaut32.lib
    LIBS += uuid.lib
    LIBS += odbc32.lib
    LIBS += odbccp32.lib
}

# Configured for the KTH CSC Karmosin computer halls
unix {
    CHAI3D = /opt/chai3d/3.2.0

    PRE_TARGETDEPS += $${CHAI3D}/lib/release/lin-x86_64-cc/libchai3d.a
    PRE_TARGETDEPS += $${CHAI3D}/extras/GLFW/lib/release/lin-x86_64-cc/libglfw.a

    INCLUDEPATH += $${CHAI3D}/src
    INCLUDEPATH += $${CHAI3D}/external/Eigen
    INCLUDEPATH += $${CHAI3D}/external/glew/include
    INCLUDEPATH += $${CHAI3D}/extras/GLFW/include

    DEFINES += LINUX
    QMAKE_CXXFLAGS += -std=c++0x
    LIBS += -L$${CHAI3D}/external/DHD/lib/lin-x86_64/
    LIBS += -L$${CHAI3D}/build/extras/GLFW
    LIBS += -L$${CHAI3D}/build
    LIBS += -L$${CHAI3D}/lib/release/lin-x86_64-cc/ -lchai3d
    LIBS += -ldrd
    LIBS += -lpthread
    LIBS += -lrt
    LIBS += -ldl
    LIBS += -lGL
    LIBS += -lGLU
    LIBS += -lusb-1.0
    LIBS += -L$${CHAI3D}/extras/GLFW/lib/release/lin-x86_64-cc/ -lglfw
    LIBS += -lX11
    LIBS += -lXcursor
    LIBS += -lXrandr
    LIBS += -lXinerama
}




