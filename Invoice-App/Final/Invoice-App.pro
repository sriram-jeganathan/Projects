QT       += core gui widgets sql

CONFIG   += c++17

TARGET    = Invoice-App
TEMPLATE  = app

# Treat new Qt deprecations as warnings, not hard errors.
DEFINES  += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    auth/authservice.cpp \
    auth/loginoverlay.cpp \
    database/databasemanager.cpp \
    pages/dashboardpage.cpp \
    pages/clientspage.cpp \
    pages/invoicespage.cpp \
    pages/reportspage.cpp \
    pages/settingspage.cpp

HEADERS += \
    mainwindow.h \
    auth/authservice.h \
    auth/loginoverlay.h \
    database/databasemanager.h \
    pages/dashboardpage.h \
    pages/clientspage.h \
    pages/invoicespage.h \
    pages/reportspage.h \
    pages/settingspage.h
