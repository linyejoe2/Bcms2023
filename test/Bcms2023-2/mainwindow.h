#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define _USE_MATH_DEFINES

#include <QMainWindow>

#include <QList>

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

public slots:
	/**
	 *  添加矢量圖層
	 *
	 * @return void
	 **/
	void addVectorLayers();

private:
	Ui::MainWindow *ui;

	QgsMapCanvas *mapCanvas;											// 地图画布
	QgsLayerTreeView *layerList;									// 圖層列表
	QgsLayerTreeMapCanvasBridge *layerListBridge; // 圖層列表資料橋接

	QDockWidget *layerOrderDock;

	QList<QgsMapLayer *> mapCanvasLayerSet; // 地图画布所用的图层集合

	/**
	 *  初始化圖層管理器
	 *
	 * @return void
	 **/
	void initLayerTreeView();
};
#endif // MAINWINDOW_H
