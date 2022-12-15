#define _USE_MATH_DEFINES
#include "bcmsmainwindow.h"


#include <QApplication>

#include <QMainWindow>
#include <QtWidgets/QApplication>
#include <QLocale>
#include <QTranslator>

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgsmaptoolpan.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BcmsMainWindow w;

    QMainWindow main_window;

    QgsMapCanvas map_canvas;                  // 實例化畫布
    QgsRasterLayer layer(R"(J:\HYP_LR.tif)"); // 載入地圖檔
    map_canvas.setLayers({&layer});           // 把地圖塞進畫布
    map_canvas.setExtent(layer.extent());
    map_canvas.refreshAllLayers(); // 繪製

    w.setCentralWidget(&map_canvas);
    w.show();
    return a.exec();
}
