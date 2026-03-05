QT += core widgets

CONFIG += c++17
TARGET = kforge-studio
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    SidebarWidget.cpp \
    BuildJobWidget.cpp \
    OtherWidgets.cpp

HEADERS += \
    BuildJob.h \
     \
    MainWindow.h \
    SidebarWidget.h \
    BuildJobWidget.h \
    OtherWidgets.h

# suppress deprecation warnings
DEFINES += QT_DEPRECATED_WARNINGS
