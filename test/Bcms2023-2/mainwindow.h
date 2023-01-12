#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class QgsAppMapTools;
class QgsMapLayer;
class QgsDataSourceManagerDialog;
class QgsSettings;

#include <QList>
#include <QMainWindow>
#include <QString>

// QGis include
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgslayertreeview.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsuserinputwidget.h>
// #include <qgsapp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget);

    //! 有什麼圖層
    enum LayerTypes { SRL, RE, AL, IC, BU, FA, NR, BA, RA };

    void addVectorLayers(QString filePath, QString DisplayName,
                         LayerTypes layerTypes);

    //! Adds a widget to the user input tool bar
    void addUserInputWidget(QWidget *widget);

    // ! 單例模式傳出主介面
    static MainWindow *instance() { return sInstance; }

    // 訊息視窗
    QgsMessageBar *messageBar();

    /**
     * Freezes all map canvases (or thaws them if the \a frozen argument is
     * FALSE).
     */
    void freezeCanvases(bool frozen = true);

    // ! 回傳地圖
    QgsMapCanvas *rMapCanvas();

    /**
     * Access the vector layer tools. This will be an instance of {\see
     * QgsGuiVectorLayerTools} by default. \returns  The vector layer tools
     */
    QgsVectorLayerTools *vectorLayerTools() { return; }

    // ! Returns a list of all map canvases open in the app.
    QList<QgsMapCanvas *> mapCanvases();

    //! Returns the CAD dock widget
    QgsAdvancedDigitizingDockWidget *cadDockWidget() {
        return mAdvancedDigitizingDockWidget;
    }

    /**
     * Creates a default attribute editor context using the main map canvas and
     * the main edit tools and message bar \since QGIS 3.12
     */
    QgsAttributeEditorContext createAttributeEditorContext();

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
    // ! mark project dirty
    void markDirty();
    // ! Hides any layers which are not selected
    void hideDeselectedLayers();
    // ! activates the add feature tool
    void addFeature();

    // ! 開啟/關閉編輯模式
    void toggleEditing();
    // ! starts/stops editing mode of a layer
    bool toggleEditing(QgsVectorLayer *vlayer, bool allowCancel = true);
    // ! 開啟/關閉所有圖層編輯相關的按鈕
    // void activateDeactivateLayerRelatedActions(QgsMapLayer *layer);
    /**
     * @brief ensure that the correct label tool buttons are enabled or disabled
     * in the user interface.
     *
     * @return ** void
     * @todo 目前沒有功能，因為也沒有 label 要處理，暫時寫出來讓別人有東西接
     */
    void updateLabelToolButtons();
    /**
     * @brief Update gui actions/menus when layers are modified
     *
     * @return void
     * @todo 目前沒功能
     * @date 12/28/2022
     * @author Randy Lin
     *
     */
    void updateLayerModifiedActions(){};

    /**
     * @brief 如果有 datasource manager dialog 的話就回傳他的 messageBar
     * 但如果沒有的話就回傳原本的 message bar.
     * Returns the message bar of the datasource manager dialog if it is
     * visible, the canvas's message bar otherwise.
     *
     * @date 12/28/2022
     * @author Randy Lin
     * @return QgsMessageBar*
     * @todo 把 datasourcce manager 整個拔掉了
     */
    QgsMessageBar *visibleMessageBar();

    //! 開啟編輯視窗
    void openEditMapDockWidget();
    //! Alerts user when commit errors occurred
    void commitError(QgsVectorLayer *vlayer,
                     const QStringList &commitErrorsList = QStringList());

    //! Returns the active map layer.
    QgsMapLayer *activeLayer();

    /**
     * Register a new application map tool \a handler, which can be used to
     * automatically setup all connections and logic required to switch to a
     * custom map tool whenever the state of the QGIS application permits.
     *
     * \note Ownership of \a handler is not transferred, and the handler must
     *       be unregistered when plugin is unloaded.
     *
     * \see QgsAbstractMapToolHandler
     * \see unregisterMapToolHandler()
     */
    //    void registerMapToolHandler(QgsAbstractMapToolHandler *handler);

   signals:
    /**
     * Emitted when a project file is successfully read
     * \note This is useful for plug-ins that store properties with project
     * files.  A plug-in can connect to this signal.  When it is emitted, the
     * plug-in knows to then check the project properties for any relevant
     * state.
     */
    void projectRead();

   private:
    void switchToMapToolViaHandler();

    Ui::MainWindow *ui;

    QgsMapCanvas *mapCanvas;                       // 地图画布
    QgsLayerTreeView *layerList;                   // 圖層列表
    QgsLayerTreeMapCanvasBridge *layerListBridge;  // 圖層列表資料橋接

    QDockWidget *layerListDock;   // 圖層列表
    QDockWidget *layerOrderDock;  // 圖層右鍵選單

    QList<QgsMapLayer *> mapCanvasLayerSet;  // 地图画布所用的图层集合

    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;

    std::unique_ptr<QgsAppMapTools> mMapTools;

    // 訊息視窗
    QgsMessageBar *mInfoBar = nullptr;

    // 會跳出來的功能視窗(長度)
    QgsUserInputWidget *mUserInputDockWidget = nullptr;

    //! Data Source Manager
    QgsDataSourceManagerDialog *mDataSourceManagerDialog = nullptr;

    //    QList<QgsAbstractMapToolHandler *> mMapToolHandlers;

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

#endif  // MAINWINDOW_H
