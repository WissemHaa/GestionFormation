QT += widgets sql charts printsupport network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connection.cpp \
    cours.cpp \
    emailservice.cpp \
    formateur.cpp \
    groqservice.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    apikeys.h \
    connection.h \
    cours.h \
    emailservice.h \
    formateur.h \
    groqservice.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore
