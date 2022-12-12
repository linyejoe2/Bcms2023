QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    main.cpp

HEADERS +=

FORMS +=

INCLUDEPATH += "$(OSGEO_HOME)\include"
INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis-rel-dev\include"
#INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis-rel-dev\include"

LIBS += -L"$(OSGEO_HOME)\apps\qgis-rel-dev\lib" -lqgis_core -lqgis_gui
LIBS += -L"$(OSGEO_HOME)\lib" -lgdal_i
#GDAL_DATA = ".\share\gdal"

TRANSLATIONS += \
    Bcms2023_zh_TW.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
