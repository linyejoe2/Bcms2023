#ifndef EDITMAPDOCKWIDGET_H
#define EDITMAPDOCKWIDGET_H

#include <QDockWidget>

// QGis include
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgslayertreeview.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsuserinputwidget.h>
// #include <qgsapp>

namespace Ui {
class EditMapDockWidget;
}

class EditMapDockWidget : public QDockWidget {
    Q_OBJECT

   public:
    explicit EditMapDockWidget(QWidget *parent = nullptr);
    ~EditMapDockWidget();

   private:
    Ui::EditMapDockWidget *ui;

    QgsMapCanvas *mapCanvas;                       // 地图画布
    QgsLayerTreeView *layerList;                   // 圖層列表
    QgsLayerTreeMapCanvasBridge *layerListBridge;  // 圖層列表資料橋接
};

#endif  // EDITMAPDOCKWIDGET_H
