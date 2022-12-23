#include "mainwindow.h"
#include "qdockwidget.h"
#include "qgslayertree.h"
#include "ui_mainwindow.h"

#include <QtDebug> // console.log()
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGridLayout>
#include <QFont>
#include <QLineEdit>
#include <QToolButton>
#include <QMargins>

// QGIS
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsproject.h>
#include <qgslayertreemodel.h>
#include <qgsapplication.h>
#include <qgslayertreelayer.h>
#include <qgslayertreegroup.h>
#include <qgslayertreeregistrybridge.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgis.h>

#include <qgspallabeling.h>					// layer label setting
#include <qgsvectorlayerlabeling.h> // layer label setting

#include "./layertreeviewmenuprovider.h"

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

	// DEMO use
	setDemo();

	// connections
	connect(ui->actionAdd_Vector, SIGNAL(triggered(bool)), this, SLOT(addVectorLayers()));
	//	connect(ui.actionAdd_Vector, SIGNAL( triggered()) ), this, SLOT(addV() ));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setDemo()
{
	// import land.
}

void MainWindow::addVectorLayers()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "", "*.shp, *.json");
	QStringList temp = filename.split(QDir::separator());
	QString basename = temp.at(temp.size() - 1);
	auto layerOptions = new QgsVectorLayer::LayerOptions(true, false);
	QgsVectorLayer *vecLayer = new QgsVectorLayer(filename, basename, "ogr", *layerOptions);
	if (!vecLayer->isValid())
	{
		QMessageBox::critical(this, "error", "layer is invalid");
		return;
	}

	auto list = vecLayer->attributeList();
	qDebug() << "randy 1: " << vecLayer->fields()[0].name();
	qDebug() << "randy 2: " << vecLayer->fields()[0].alias();
	qDebug() << "randy 3: " << vecLayer->fields()[0].comment();
	QgsPalLayerSettings layerSetting;
	layerSetting.drawLabels = true;												 // set show label or not.
	layerSetting.fieldName = vecLayer->fields()[1].name(); // set show witch property.
	layerSetting.centroidWhole = true;										 // set label show at center

	vecLayer->setLabeling(new QgsVectorLayerSimpleLabeling(layerSetting));
	vecLayer->setLabelsEnabled(true);
	// qDebug() << "randy list: " << list.takeFirst();
	// qDebug() << "linyejoe2";

	QgsProject::instance()->addMapLayer(vecLayer);
	mapCanvasLayerSet.append(vecLayer);
	mapCanvas->setExtent(vecLayer->extent());
	mapCanvas->setLayers(mapCanvasLayerSet);
	mapCanvas->setVisible(true);
	mapCanvas->freeze(false);
	mapCanvas->refresh();
}

void MainWindow::initLayerTreeView()
{
	QgsLayerTreeModel *model = new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
	model->setFlag(QgsLayerTreeModel::AllowNodeRename);
	model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
	model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
	model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
	model->setAutoCollapseLegendNodes(10);
	layerList->setModel(model);

	// 添加右键菜单
	layerList->setMenuProvider(new LayerTreeViewMenuProvider(layerList, mapCanvas));

	connect(QgsProject::instance()->layerTreeRegistryBridge(), SIGNAL(addedLayersToLayerTree(QList<QgsMapLayer *>)),
					this, SLOT(autoSelectAddedLayer(QList<QgsMapLayer *>)));

	//	// 设置这个路径是为了获取图标文件
	//	QString iconDir = "../images/themes/default/";

	//	// add group tool button
	//	QToolButton *btnAddGroup = new QToolButton();
	//	btnAddGroup->setAutoRaise(true);
	//	btnAddGroup->setIcon(QIcon(iconDir + "mActionAdd.png"));

	//	btnAddGroup->setToolTip(tr("Add Group"));
	//	connect(btnAddGroup, SIGNAL(clicked()), layerList->defaultActions(), SLOT(addGroup()));

	//	// expand / collapse tool buttons
	//	QToolButton *btnExpandAll = new QToolButton();
	//	btnExpandAll->setAutoRaise(true);
	//	btnExpandAll->setIcon(QIcon(iconDir + "mActionExpandTree.png"));
	//	btnExpandAll->setToolTip(tr("Expand All"));
	//	connect(btnExpandAll, SIGNAL(clicked()), layerList, SLOT(expandAll()));

	//	QToolButton *btnCollapseAll = new QToolButton();
	//	btnCollapseAll->setAutoRaise(true);
	//	btnCollapseAll->setIcon(QIcon(iconDir + "mActionCollapseTree.png"));
	//	btnCollapseAll->setToolTip(tr("Collapse All"));
	//	connect(btnCollapseAll, SIGNAL(clicked()), layerList, SLOT(collapseAll()));

	//	// remove item button
	//	QToolButton *btnRemoveItem = new QToolButton();
	//	// btnRemoveItem->setDefaultAction( this->m_actionRemoveLayer );
	//	btnRemoveItem->setAutoRaise(true);

	//	// 按钮布局
	//	QHBoxLayout *toolbarLayout = new QHBoxLayout();
	//	toolbarLayout->setContentsMargins(QMargins(5, 0, 5, 0));
	//	toolbarLayout->addWidget(btnAddGroup);
	//	toolbarLayout->addWidget(btnCollapseAll);
	//	toolbarLayout->addWidget(btnExpandAll);
	//	toolbarLayout->addWidget(btnRemoveItem);
	//	toolbarLayout->addStretch();

	//	QVBoxLayout *vboxLayout = new QVBoxLayout();
	//	vboxLayout->setMargin(0);
	//	vboxLayout->addLayout(toolbarLayout);
	//	vboxLayout->addWidget(layerList);

	//	// 装进dock widget中
	//	layerListDock = new QDockWidget(tr("Layers"), this);
	//	layerListDock->setObjectName("Layers");
	//	layerListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	//	QWidget *w = new QWidget();
	//	w->setLayout(vboxLayout);
	//	layerListDock->setWidget(w);
	//	addDockWidget(Qt::RightDockWidgetArea, layerListDock);

	//	// 連接畫布和圖層列表
	//	layerListBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), mapCanvas, this);
	//	connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)), layerListBridge, SLOT(writeProject(QDomDocument &)));
	//	connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)), layerListBridge, SLOT(readProject(QDomDocument)));
}

void MainWindow::autoSelectAddedLayer(QList<QgsMapLayer *> layers)
{
	if (layers.count())
	{
		QgsLayerTreeLayer *nodeLayer = QgsProject::instance()->layerTreeRoot()->findLayer(layers[0]->id());

		if (!nodeLayer)
		{
			return;
		}

		QModelIndex index = layerList->layerTreeModel()->node2index(nodeLayer);
		layerList->setCurrentIndex(index);
	}
}

void MainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
	QMainWindow::addDockWidget(area, dockwidget);
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	dockwidget->show();
	mapCanvas->refresh();
}
