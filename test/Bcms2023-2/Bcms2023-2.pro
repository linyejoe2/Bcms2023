QT       += core gui xml svg concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += _USE_MATH_DEFINES

SOURCES += \
    core/qgsfeatureaction.cpp \
    editmapdockwidget.cpp \
    layertreeviewmenuprovider.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_canvas.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_curve.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_fitter.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_grid.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_item.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_itemdict.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_layout.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_magnifier.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_marker.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_panner.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_picker.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_plot.cpp \
    lib/qwtpolar-1.1.1/qwt_polar_renderer.cpp \
    main.cpp \
    mainwindow.cpp \
    maptools/qgsappmaptools.cpp \
    maptools/qgsguivectorlayertools.cpp \
    maptools/qgsguivectorlayertools.cpp \
    maptools/qgsmaptooladdfeature.cpp \
    maptools/qgsmaptoolselect.cpp \
    maptools/qgsmaptoolselectionhandler.cpp \
    maptools/qgsmaptoolselectutils.cpp

HEADERS += \
    core/qgsfeatureaction.h \
    editmapdockwidget.h \
    layertreeviewmenuprovider.h \
    lib/json.hpp \
    lib/qwtpolar-1.1.1/qwt_polar.h \
    lib/qwtpolar-1.1.1/qwt_polar_canvas.h \
    lib/qwtpolar-1.1.1/qwt_polar_curve.h \
    lib/qwtpolar-1.1.1/qwt_polar_fitter.h \
    lib/qwtpolar-1.1.1/qwt_polar_global.h \
    lib/qwtpolar-1.1.1/qwt_polar_grid.h \
    lib/qwtpolar-1.1.1/qwt_polar_item.h \
    lib/qwtpolar-1.1.1/qwt_polar_itemdict.h \
    lib/qwtpolar-1.1.1/qwt_polar_layout.h \
    lib/qwtpolar-1.1.1/qwt_polar_magnifier.h \
    lib/qwtpolar-1.1.1/qwt_polar_marker.h \
    lib/qwtpolar-1.1.1/qwt_polar_panner.h \
    lib/qwtpolar-1.1.1/qwt_polar_picker.h \
    lib/qwtpolar-1.1.1/qwt_polar_plot.h \
    lib/qwtpolar-1.1.1/qwt_polar_renderer.h \
    mainwindow.h \
    maptools/qgsappmaptools.h \
    maptools/qgsguivectorlayertools.h \
    maptools/qgsguivectorlayertools.h \
    maptools/qgsmaptooladdfeature.h \
    maptools/qgsmaptoolselect.h \
    maptools/qgsmaptoolselectionhandler.h \
    maptools/qgsmaptoolselectutils.h

FORMS += \
    editmapdockwidget.ui \
    mainwindow.ui

## QGIS 相關
#
# qgis-ltr-dev qgis-ltr qgis-rel-dev qgis
INCLUDEPATH += "$(OSGEO_HOME)\include"
# INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis-rel-dev\include"
#INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis-ltr\include"
INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis\include"
INCLUDEPATH += "$(OSGEO_HOME)\apps\Qt5\include"
INCLUDEPATH += "$(OSGEO_HOME)\apps\Qt5\include\QtCore"
INCLUDEPATH += "$(OSGEO_HOME)\apps\Qt5\include\QtWidgets"
INCLUDEPATH += "$(OSGEO_HOME)\apps\Qt5\include\QtXml"
INCLUDEPATH += "$(OSGEO_HOME)\apps\Qt5\include\QtGui"

#LIBS += -L"$(OSGEO_HOME)\apps\qgis-ltr\lib" -lqgis_core -lqgis_gui -lqgis_app
LIBS += -L"$(OSGEO_HOME)\apps\qgis\lib" -lqgis_core -lqgis_gui -lqgis_app
LIBS += -L"$(OSGEO_HOME)\apps\Qt5\lib" -lQt5Core -lQt5Gui -lQt5Widgets
# LIBS += -L"$(OSGEO_HOME)\apps\qgis-rel-dev\lib" -lqgis_core -lqgis_gui
#
## QGIS 相關結束

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
