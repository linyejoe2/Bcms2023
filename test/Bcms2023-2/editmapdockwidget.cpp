#include "editmapdockwidget.h"

#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>

#include <QDialog>
#include <QFileDialog>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtDebug>

#include "ui_editmapdockwidget.h"

// QGIS
#include <qgis.h>
#include <qgsapplication.h>
#include <qgslayertreegroup.h>
#include <qgslayertreelayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreeregistrybridge.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgsmessagebar.h>
#include <qgsproject.h>
#include <qgsrasterlayer.h>
#include <qgsuserinputwidget.h>
#include <qgsvectorlayer.h>
#include <qshortcut.h>

// START layer label setting
#include <qgspallabeling.h>
#include <qgsvectorlayerlabeling.h>
// END layer label setting

// START layer symbol setting
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgsmarkersymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>
// END layer symbol setting

EditMapDockWidget::EditMapDockWidget(QWidget *parent)
    : QDockWidget(parent), ui(new Ui::EditMapDockWidget) {
    ui->setupUi(this);

    // 實例化畫布
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setVisible(true);

    // layerOptions.
    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};

    // 1. import land.
    QgsVectorLayer *landLayer =
        new QgsVectorLayer("./temp/400_0044_land.json", QStringLiteral("地籍"),
                           "ogr", layerOptions);

    // 1-1. set land symbol to blue padding and transparent fill.
    QgsSimpleFillSymbolLayer *landSymbolLayer = new QgsSimpleFillSymbolLayer();
    landSymbolLayer->setStrokeColor(Qt::blue);
    landSymbolLayer->setColor(Qt::transparent);

    QgsFillSymbol *fillSymbol = new QgsFillSymbol();
    fillSymbol->changeSymbolLayer(0, landSymbolLayer);

    landLayer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));
    // END of set land symbol

    // 1-2. set land label to land-no
    QgsPalLayerSettings landLayerLabelSetting;
    landLayerLabelSetting.fieldName = landLayer->fields()[0].name();
    landLayerLabelSetting.drawLabels = true;
    landLayerLabelSetting.centroidWhole = true;

    landLayer->setLabeling(
        new QgsVectorLayerSimpleLabeling(landLayerLabelSetting));
    landLayer->setLabelsEnabled(true);
    // END of set land label

    mapCanvas->setExtent(landLayer->extent());
    mapCanvas->setLayers({landLayer});
    mapCanvas->setVisible(true);
    mapCanvas->freeze(false);
    mapCanvas->refresh();

    // QgsRasterLayer layer(R"(J:\HYP_LR.tif)"); // 載入地圖檔
    // mapCanvas->setLayers({&layer});           // 把地圖塞進畫布
    // mapCanvas->setExtent(layer.extent());
    // mapCanvas->refreshAllLayers(); // 繪製

    this->setWidget(mapCanvas);

    // layerList = new QgsLayerTreeView(this);
    // layerList->setObjectName(QStringLiteral("theLayerTreeView"));
    // initLayerTreeView();
}

EditMapDockWidget::~EditMapDockWidget() { delete ui; }
