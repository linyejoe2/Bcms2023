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
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	/**
	 *  添加矢量图层
	 *
	 * @return void
	**/
	void addVectorLayers();

private slots:
//	void on_actionAdd_Vector_triggered();

private:
	Ui::MainWindow *ui;

	QgsMapCanvas* mapCanvas; // 地图画布
	QList<QgsMapLayer*> mapCanvasLayerSet; // 地图画布所用的图层集合
	QgsLayerTreeView* layerList; // 圖層列表
	QgsLayerTreeMapCanvasBridge* layerListBridge; // 圖層列表資料橋接

	void initLayerTreeView();
};
#endif // MAINWINDOW_H
