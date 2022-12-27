#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class QgsAppMapTools;

#include <QMainWindow>
#include <QList>
#include <QString>

// QGis include
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgslayertreeview.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgsuserinputwidget.h>
// #include <qgsapp>

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

	//! 有什麼圖層
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

	void addVectorLayers(QString filePath, QString DisplayName, LayerTypes layerTypes);

	//! Adds a widget to the user input tool bar
	void addUserInputWidget(QWidget *widget);

	// ! 單例模式傳出主介面
	static MainWindow *instance() { return sInstance; }

	// 訊息視窗
	QgsMessageBar *messageBar();

public slots:
	// ! 添加矢量圖層
	void addVectorLayers();

	void autoSelectAddedLayer(QList<QgsMapLayer *> layers);

	// Map tools
	// ! 移動圖層
	void panMap();
	// ! 放大圖層
	void zoomIn();
	// ! 縮小圖層
	void zoomOut();
	// ! 縮放至整個圖層
	void zoomFull();
	// ! 縮放至選擇的圖層
	void zoomToSelected();
	// ! 選擇元素
	void selectFeatures();
	// ! 控制表單選擇元素
	void changeSelectLayer(QgsMapLayer *layer);
	//! mark project dirty
	void markDirty();
	//! Hides any layers which are not selected
	void hideDeselectedLayers();

private:
	Ui::MainWindow *ui;

	QgsMapCanvas *mapCanvas;											// 地图画布
	QgsLayerTreeView *layerList;									// 圖層列表
	QgsLayerTreeMapCanvasBridge *layerListBridge; // 圖層列表資料橋接

	QDockWidget *layerListDock;	 // 圖層列表
	QDockWidget *layerOrderDock; // 圖層右鍵選單

	QList<QgsMapLayer *> mapCanvasLayerSet; // 地图画布所用的图层集合

	QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;

	std::unique_ptr<QgsAppMapTools> mMapTools;

	// 訊息視窗
	QgsMessageBar *mInfoBar = nullptr;

	// 會跳出來的功能視窗(長度)
	QgsUserInputWidget *mUserInputDockWidget = nullptr;

	/**
	 *  初始化圖層管理器
	 *
	 * @return void
	 **/
	void initLayerTreeView();

	// ! 設定成樣板模式
	void setDemo();

	static MainWindow *sInstance;
};

#endif // MAINWINDOW_H
