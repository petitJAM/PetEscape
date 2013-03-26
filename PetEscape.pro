TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

include(PetEscape.mine.pro)

win32 {
    LIBS += libboost_system-vc100-mt-gd-1_53.lib
}

unix {
    LIBS += # Unix LIbs.
}

INCLUDEPATH += include

SOURCES += \
    src/petescape/networking/common/TCP_Connection.cpp \
    src/petescape/networking/common/NetCodeParser.cpp \
    src/petescape/networking/common/NetCodeGenerator.cpp \
    src/petescape/networking/client/ClientConnection.cpp \
    src/petescape/networking/server/ServerConnection.cpp \
    src/petescape/core/main.cpp \
    src/petescape/core/client/c_main.cpp \
    src/petescape/core/server/s_main.cpp

HEADERS += \
    include/petescape/networking/client/ClientConnection.h \
    include/petescape/networking/common/TCP_Connection.h \
    include/petescape/networking/common/NetCodeParser.h \
    include/petescape/networking/common/NetCodeGenerator.h \
    include/petescape/networking/common/net_struct.h \
    include/petescape/networking/server/ServerConnection.h \
    include/petescape/core/server/server.h \
    include/petescape/core/client/client.h
