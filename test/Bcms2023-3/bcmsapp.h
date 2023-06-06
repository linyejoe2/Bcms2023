﻿#ifndef BCMSAPP_H
#define BCMSAPP_H

class QgsAppMapTools;
class QgsGuiVectorLayerTools;
class BcmsLoadForm;

#include <QMainWindow>

// Qgis
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgsattributeeditorcontext.h>
#include <qgsbrowsertreeview.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgslayertreeview.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmessagebar.h>
#include <qgsraster.h>
#include <qgsscalecombobox.h>
#include <qgsvectorlayertools.h>

#include "bcmsloadform.h"
#include "core/const.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class BcmsApp;
}
QT_END_NAMESPACE

class BcmsApp : public QMainWindow {
    Q_OBJECT

   public:
    BcmsApp(QWidget *parent = nullptr);
    ~BcmsApp();

    //! 套繪系統的圖層
    enum LayerTypes {
        LAND,  // 地籍
        SRL,
        RE,
        AL,  // 法定騎樓
        IC,
        BU,  // 建築物
        FA,
        NR,
        BA,  // 法定空地
        RA,
        OTHER  // 無法辨識或是未定義的，都會被放到此類
    };

    //! 傳出畫布
    QgsMapCanvas *mapCanvas() { return mMapCanvas; }
    //! 傳出訊息列
    QgsMessageBar *messageBar() { return mMessageBar; }
    //! 傳出 CAD 工具
    QgsAdvancedDigitizingDockWidget *cadDockWidget() {
        return mAdvancedDigitizingDockWidget;
    }
    QgsVectorLayerTools *vectorLayerTools() { return mVectorLayerTools; }
    //! 傳出單例類別
    static BcmsApp *instance() { return sInstance; }
    /**
     * Creates a default attribute editor context using the main map canvas and
     * the main edit tools and message bar \since QGIS 3.12
     */
    QgsAttributeEditorContext createAttributeEditorContext();
    //! 凍結畫布或解凍
    void freezeCanvases(bool frozen = true) { mMapCanvas->freeze(frozen); }

   public slots:
    void log();
    //! 切換編輯模式
    void toggleEditing();
    //! 新增一個圖層
    void addFeature();
    //! 自動選擇圖層
    void autoSelectAddedLayer(QList<QgsMapLayer *> layers);
    //! 切換選擇的圖層
    void changeSelectLayer(QgsMapLayer *layer);
    //! 選擇元素
    void selectFeatures();
    //! 刪除選擇元素
    void deleteSelected(QgsMapLayer *layer = nullptr, QWidget *parent = nullptr,
                        bool checkFeaturesVisible = false);
    //! 開啟地段選擇畫面
    void openMBcmsLoadForm();

   private:
    Ui::BcmsApp *ui;

    //! 畫布
    QgsMapCanvas *mMapCanvas;
    //! 圖層管理器
    QgsLayerTreeView *mLayerTreeView;
    //! 圖層列表
    QList<QgsMapLayer *> mMapCanvasLayers;
    //! 畫布與圖層管理器的連接器
    QgsLayerTreeMapCanvasBridge *mLayerTreeCanvasBridge;
    //! 訊息列
    QgsMessageBar *mMessageBar;
    //! 工具列
    std::unique_ptr<QgsAppMapTools> mMapTools;
    //! CAD 工具
    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget;
    //! 矢量工具
    QgsVectorLayerTools *mVectorLayerTools;
    //! 圖層選擇畫面
    BcmsLoadForm *mBcmsLoadForm;
    //! 單例這個類
    static BcmsApp *sInstance;

    //! 添加圖層
    void addVectorLayers(QString filePath, QString DisplayName,
                         LayerTypes layerTypes);
    //! 初始化圖層管理器
    void initLayerTreeView();

    //! 載入某段
    void loadLand(const ILandCode &landCode);
    //! 載入某段的套繪資料
    void loadBuilding(const ILandCode &landCode);
    //! 定位到某個地號
    void setLocate(const ILandCode &landCode);

    //! 清理地圖
    void resetMap();

    //! 檢查 GeoJSON 文件
    bool checkGeoJSON(const QString &filePath);

    //! 尋找圖層
    QgsVectorLayer *findLayerByName(const QString &name);
};
#endif  // BCMSAPP_H
