TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

include(PetEscape.mine.pro)

win32 {
    LIBS += libboost_system-vc100-mt-1_53.lib \
            -lallegro-5.0.8-mt \
            -lallegro_image-5.0.8-mt \
            -lallegro_font-5.0.8-mt \
            -lallegro_primitives-5.0.8-mt \
            -lallegro_ttf-5.0.8-mt \

}

unix {
    LIBS += # Unix LIbs.
}

INCLUDEPATH += include

SOURCES += \
    src/launcher.cpp \
    src/petescape/core/main.cpp \
    src/petescape/core/client/c_main.cpp \
    src/petescape/core/server/s_main.cpp \
    src/petescape/core/GameObject.cpp \
    src/petescape/core/ObjectRenderer.cpp \
    src/petescape/core/GameMap.cpp \
    src/petescape/core/Block.cpp \
    src/petescape/core/BlockMap.cpp

HEADERS += \
    include/launcher.h \
    include/petescape/networking/common/net_struct.h \
    include/petescape/core/server/server.h \
    include/petescape/core/client/client.h \
    include/petescape/core/client/client_resources.h \
    include/petescape/core/GameObject.h \
    include/petescape/core/ObjectRenderer.h \
    include/petescape/core/core_defs.h \
    include/petescape/core/GameMap.h \
    include/petescape/core/Block.h \
    include/petescape/core/BlockMap.h
