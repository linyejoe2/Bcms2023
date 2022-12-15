#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

// QGIS
#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgsmaptoolpan.h>
#include <qgsvectorlayer.h>
#include <qgslayertreeview.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgsproject.h>
#include <qgis.h>

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// 實例化畫布
	mapCanvas = new QgsMapCanvas();
	mapCanvas->setVisible(true);

	// 實例化並設定圖層管理器
	layerList = new QgsLayerTreeView(this);
	initLayerTreeView();

	// layout
	this->setCentralWidget(mapCanvas);
	ui->LayersList->setWidget(layerList);

	//	connections
	connect(ui->actionAdd_Vector, SIGNAL(triggered(bool)), this, SLOT(addVectorLayers()));
	//	connect(ui.actionAdd_Vector, SIGNAL( triggered()) ), this, SLOT(addV() ));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::addVectorLayers()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "", "*.shp, *.json");
	QStringList temp = filename.split(QDir::separator());
	QString basename = temp.at(temp.size() - 1);
	auto layerOptions = new QgsVectorLayer::LayerOptions(true,false);
	QgsVectorLayer* vecLayer = new QgsVectorLayer(filename, basename, "ogr", *layerOptions);
	if (!vecLayer->isValid())
	{
		QMessageBox::critical(this, "error", "layer is invalid");
		return;
	}

	QgsProject::instance()->addMapLayer(vecLayer);
	mapCanvasLayerSet.append(vecLayer);
	mapCanvas->setExtent(vecLayer->extent());
	mapCanvas->setLayers(mapCanvasLayerSet);
	mapCanvas->setVisible(true);
	mapCanvas->freeze(false);
	mapCanvas->refresh();
}

void MainWindow::initLayerTreeView(){
	QgsLayerTreeModel* model = new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), this );
	model->setFlag( QgsLayerTreeModel::AllowNodeRename );
	model->setFlag( QgsLayerTreeModel::AllowNodeReorder );
	model->setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility );
	model->setFlag( QgsLayerTreeModel::ShowLegendAsTree );
	model->setAutoCollapseLegendNodes( 10 );
	layerList->setModel( model );

	// 连接地图画布和图层管理器
	layerListBridge = new QgsLayerTreeMapCanvasBridge( QgsProject::instance()->layerTreeRoot(), mapCanvas, this );
	connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ), layerListBridge, SLOT( writeProject( QDomDocument& ) ) );
	connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ), layerListBridge, SLOT( readProject( QDomDocument ) ) );

}
