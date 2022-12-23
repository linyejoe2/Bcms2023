#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define _USE_MATH_DEFINES

#include <QMainWindow>

#include <QList>
#include <QString>

// QGis include
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgslayertreeview.h>
#include <qgslayertreemapcanvasbridge.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget);

	enum LayerTypes
	{
		SRL,
		RE,
		AL,
		IC,
		BU,
		FA,
		NR,
		BA,
		RA
	};

public slots:
	//! 添加矢量圖層
	void addVectorLayers();
	void addVectorLayers(QString filePath, QString DisplayName, LayerTypes layerTypes);

	void autoSelectAddedLayer(QList<QgsMapLayer *> layers);

private:
	Ui::MainWindow *ui;

	QgsMapCanvas *mapCanvas;											// 地图画布
	QgsLayerTreeView *layerList;									// 圖層列表
	QgsLayerTreeMapCanvasBridge *layerListBridge; // 圖層列表資料橋接

	QDockWidget *layerListDock;	 // 圖層列表
	QDockWidget *layerOrderDock; // 圖層右鍵選單

	QList<QgsMapLayer *> mapCanvasLayerSet; // 地图画布所用的图层集合

	/**
	 *  初始化圖層管理器
	 *
	 * @return void
	 **/
	void initLayerTreeView();

	// ! 設定成樣板模式
	void setDemo();
};

#endif // MAINWINDOW_H
