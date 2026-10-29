QT += core gui sql widgets printsupport

CONFIG += c++17

msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    employe.cpp \
    conge.cpp \
    historiqueemploye.cpp \
    modelemployes.cpp \
    dialogemploye.cpp \
    dialogconge.cpp \
    chartsevolution.cpp \
    chartsrepartition.cpp \
    chartsbarres.cpp \
    kpicard.cpp \
    vueemployes.cpp \
    fichedetailleemploye.cpp \
    vueaccueil.cpp \
    vueservices.cpp \
    vuerapports.cpp

HEADERS += \
    employe.h \
    conge.h \
    historiqueemploye.h \
    modelemployes.h \
    deleguesemployes.h \
    dialogemploye.h \
    dialogconge.h \
    chartsevolution.h \
    chartsrepartition.h \
    chartsbarres.h \
    kpicard.h \
    icones.h \
    outilsemployes.h \
    stylesemployes.h \
    connection.h \
    mainwindow.h \
    vueemployes.h \
    fichedetailleemploye.h \
    vueaccueil.h \
    vueservices.h \
    vuerapports.h

FORMS += \
    dialogemploye.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
