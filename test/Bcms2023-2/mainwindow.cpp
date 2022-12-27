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
#include <qtextcodec.h>

// QGIS
#include <qgsapplication.h>
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
#include <qshortcut.h>
#include <qgsuserinputwidget.h>
#include <qgsmessagebar.h>

// layer label setting
#include <qgspallabeling.h>
#include <qgsvectorlayerlabeling.h>
// END of layer label setting

// layer symbol setting
#include <qgsmarkersymbollayer.h>
#include <qgssymbol.h>
#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsadvanceddigitizingdockwidget.h>
// END of layer symbol setting

#include "layertreeviewmenuprovider.h"
#include "maptools/qgsappmaptools.h"

// 單例模式指向自己
MainWindow *MainWindow::sInstance = nullptr;

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// 實例化畫布
	mapCanvas = new QgsMapCanvas();
	mapCanvas->setVisible(true);

	// 實例化並設定圖層管理器
	layerList = new QgsLayerTreeView(this);
	layerList->setObjectName(QStringLiteral("theLayerTreeView"));
	initLayerTreeView();

	// layout
	this->setCentralWidget(mapCanvas);
	ui->LayersList->setWidget(layerList);

	// DEMO use
	setDemo();

	// msg bar
	QWidget *centralWidget = this->centralWidget();
	mInfoBar = new QgsMessageBar(centralWidget);
	layerList->setMessageBar(mInfoBar);

	// Advanced Digitizing dock
	mAdvancedDigitizingDockWidget = new QgsAdvancedDigitizingDockWidget(mapCanvas, this);
	mAdvancedDigitizingDockWidget->setWindowTitle(tr("Advanced Digitizing"));
	mAdvancedDigitizingDockWidget->setObjectName(QStringLiteral("AdvancedDigitizingTools"));

	QShortcut *showAdvancedDigitizingDock = new QShortcut(QKeySequence(tr("Ctrl+4")), this);
	connect(showAdvancedDigitizingDock, &QShortcut::activated, mAdvancedDigitizingDockWidget, &QgsDockWidget::toggleUserVisible);
	showAdvancedDigitizingDock->setObjectName(QStringLiteral("ShowAdvancedDigitizingPanel"));
	showAdvancedDigitizingDock->setWhatsThis(tr("Show Advanced Digitizing Panel"));
	mMapTools = std::make_unique<QgsAppMapTools>(mapCanvas, mAdvancedDigitizingDockWidget);
	addDockWidget(Qt::LeftDockWidgetArea, mAdvancedDigitizingDockWidget);
	mAdvancedDigitizingDockWidget->show();

	// User Input Dock Widget
	mUserInputDockWidget = new QgsUserInputWidget(mapCanvas);
	mUserInputDockWidget->setObjectName(QStringLiteral("UserInputDockWidget"));
	mUserInputDockWidget->setAnchorWidget(mapCanvas);
	mUserInputDockWidget->setAnchorWidgetPoint(QgsFloatingWidget::TopRight);
	mUserInputDockWidget->setAnchorPoint(QgsFloatingWidget::TopRight);

	// connections
	connect(ui->actionAdd_Vector, SIGNAL(triggered(bool)), this, SLOT(addVectorLayers()));
	connect(ui->actionPan_Map, SIGNAL(triggered()), this, SLOT(panMap()));
	connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));
	connect(ui->actionZoom_To_Selected, SIGNAL(triggered()), this, SLOT(zoomToSelected()));
	connect(ui->actionSelect_Rectangle, SIGNAL(triggered()), this, SLOT(selectFeatures()));
	connect(ui->actionZoom_Full_Map, SIGNAL(triggered()), this, SLOT(zoomFull()));
	connect(layerList, SIGNAL(currentLayerChanged(QgsMapLayer *)), this, SLOT(changeSelectLayer(QgsMapLayer *)));
	connect(layerList->layerTreeModel()->rootGroup(), &QgsLayerTreeNode::visibilityChanged,
					this, &MainWindow::markDirty);
	// connect(layerList->selectionModel(), SIGNAL(QItemSelectionModel::selectionChanged()), this, SLOT(changeSelectLayer()));

	//	connect(ui.actionAdd_Vector, SIGNAL( triggered()) ), this, SLOT(addV() ));
}

MainWindow::~MainWindow()
{
	delete ui;
}

QgsMessageBar *MainWindow::messageBar()
{
	// Q_ASSERT( mInfoBar );
	return mInfoBar;
}

void MainWindow::zoomFull()
{
	mapCanvas->zoomToProjectExtent();
}

void MainWindow::markDirty()
{
	qDebug() << "linyejoe2: ";
	// notify the project that there was a change
	QgsProject::instance()->setDirty(true);
	// hideDeselectedLayers();
}

// void QgisApp::hideAllLayers()
// {
//   QgsDebugMsgLevel( QStringLiteral( "hiding all layers!" ), 3 );

//   const auto constChildren = mLayerTreeView->layerTreeModel()->rootGroup()->children();
//   for ( QgsLayerTreeNode *node : constChildren )
//   {
//     node->setItemVisibilityCheckedRecursive( false );
//   }
// }

// void QgisApp::showAllLayers()
// {
//   QgsDebugMsgLevel( QStringLiteral( "Showing all layers!" ), 3 );
//   mLayerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive( true );
// }

void MainWindow::hideDeselectedLayers()
{
	QList<QgsLayerTreeLayer *> selectedLayerNodes = layerList->selectedLayerNodes();

	const auto constFindLayers = layerList->layerTreeModel()->rootGroup()->findLayers();
	for (QgsLayerTreeLayer *nodeLayer : constFindLayers)
	{
		if (selectedLayerNodes.contains(nodeLayer))
			continue;
		nodeLayer->setItemVisibilityChecked(false);
	}
}

void MainWindow::changeSelectLayer(QgsMapLayer *layer)
{
	if (!layer)
	{
		return;
	}
	if (mapCanvas)
	{
		mapCanvas->setCurrentLayer(layer);
	}
	// const QList<QgsLayerTreeLayer *> selectedLayers = layerList ? layerList->selectedLayerNodes() : QList<QgsLayerTreeLayer *>();

	// qDebug() << "linyejoe1: " << selectedLayers;
	// qDebug() << "linyejoe2: " << layer;

	// mapCanvas->setCurrentLayer(layer);
}

void MainWindow::selectFeatures()
{
	mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::SelectFeatures));
}

void MainWindow::panMap()
{
	mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::Pan));
}

void MainWindow::zoomIn()
{
	mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::ZoomIn));
}

void MainWindow::zoomOut()
{
	mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::ZoomOut));
}

void MainWindow::zoomToSelected()
{
	const QList<QgsMapLayer *> layers = layerList->selectedLayers();

	if (layers.size() > 1)
		mapCanvas->zoomToSelected(layers);
	else
		mapCanvas->zoomToSelected();
}

void MainWindow::addUserInputWidget(QWidget *widget)
{
	mUserInputDockWidget->addUserInputWidget(widget);
}

void MainWindow::setDemo()
{
	addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定空地"), BA);
	addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定騎樓"), AL);
	addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("建築物"), BU);

	// layerOptions.
	const QgsVectorLayer::LayerOptions layerOptions{QgsProject::instance()->transformContext()};

	// 1. import land.
	QgsVectorLayer *landLayer = new QgsVectorLayer(
			"./temp/400_0044_land.json",
			QStringLiteral("地籍"),
			"ogr",
			layerOptions);

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

	landLayer->setLabeling(new QgsVectorLayerSimpleLabeling(landLayerLabelSetting));
	landLayer->setLabelsEnabled(true);
	// END of set land label

	QgsProject::instance()
			->addMapLayer(landLayer);
	mapCanvasLayerSet.append(landLayer);
	mapCanvas->setExtent(landLayer->extent());
	mapCanvas->setLayers(mapCanvasLayerSet);
	mapCanvas->setVisible(true);
	mapCanvas->freeze(false);
	mapCanvas->refresh();
}

void MainWindow::addVectorLayers(QString filePath, QString DisplayName, LayerTypes layerTypes)
{
	// layerOptions.
	const QgsVectorLayer::LayerOptions layerOptions{QgsProject::instance()->transformContext()};

	// 1. import layer.
	QgsVectorLayer *layer = new QgsVectorLayer(
			filePath,
			DisplayName,
			"ogr",
			layerOptions);

	QgsSimpleFillSymbolLayer *landSymbolLayer = new QgsSimpleFillSymbolLayer();
	QgsFillSymbol *fillSymbol = new QgsFillSymbol();
	QgsPalLayerSettings layerLabelSetting;

	// 2. layer setting.
	switch (layerTypes)
	{
	case BU:
		layer->setSubsetString("\"layer\" = 'BU'");

		landSymbolLayer->setColor(Qt::red);
		fillSymbol->changeSymbolLayer(0, landSymbolLayer);
		layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));

		layerLabelSetting.fieldName = layer->fields()[0].name();
		layerLabelSetting.drawLabels = true;
		layerLabelSetting.centroidWhole = true;
		layer->setLabeling(new QgsVectorLayerSimpleLabeling(layerLabelSetting));
		layer->setLabelsEnabled(true);

		mapCanvas->setCurrentLayer(layer);
		break;
	case BA:
		layer->setSubsetString("\"layer\" = 'BA'");

		landSymbolLayer->setColor(Qt::darkGreen);
		fillSymbol->changeSymbolLayer(0, landSymbolLayer);
		layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));
		break;
	case AL:
		layer->setSubsetString("\"layer\" = 'AL'");

		landSymbolLayer->setColor(Qt::yellow);
		fillSymbol->changeSymbolLayer(0, landSymbolLayer);
		layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));
		break;
	default:
		break;
	}

	QgsProject::instance()->addMapLayer(layer);
	mapCanvasLayerSet.append(layer);
	mapCanvas->setExtent(layer->extent());
	mapCanvas->setLayers(mapCanvasLayerSet);
	mapCanvas->setVisible(true);
	mapCanvas->freeze(false);
	mapCanvas->refresh();
}

void MainWindow::addVectorLayers()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "", "*.shp, *.json");
	QStringList temp = filename.split(QDir::separator());
	QString basename = temp.at(temp.size() - 1);
	// auto layerOptions = new QgsVectorLayer::LayerOptions(true, false);

	const QgsVectorLayer::LayerOptions layerOptions{QgsProject::instance()->transformContext()};
	QgsVectorLayer *vecLayer = new QgsVectorLayer(filename, basename, "ogr", layerOptions);
	if (!vecLayer->isValid())
	{
		QMessageBox::critical(this, "error", "layer is invalid");
		return;
	}

	auto list = vecLayer->attributeList();
	QgsPalLayerSettings layerSetting;
	// layerSetting.drawLabels = true;												 // set show label or not.
	// layerSetting.fieldName = vecLayer->fields()[1].name(); // set show witch property.
	// layerSetting.centroidWhole = true;										 // set label show at center

	// vecLayer->setLabeling(new QgsVectorLayerSimpleLabeling(layerSetting));
	// vecLayer->setLabelsEnabled(true);
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
	layerListBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), mapCanvas, this);
	connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)), layerListBridge, SLOT(writeProject(QDomDocument &)));
	connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)), layerListBridge, SLOT(readProject(QDomDocument)));
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
