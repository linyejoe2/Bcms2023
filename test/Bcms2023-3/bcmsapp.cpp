#include "bcmsapp.h"

#include "ui_bcmsapp.h"

// START QT include
#include <qcheckbox.h>
#include <qgridlayout.h>
#include <qmessagebox.h>
#include <qshortcut.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// START QGis include
//  #include <qgsmessagebar.h>
//  #include <qgsapplication.h>
//  #include <qgsattributedialog.h>
#include <qgsdefaultvalue.h>
#include <qgslayertreegroup.h>
#include <qgslayertreelayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreeregistrybridge.h>
#include <qgslayertreeviewdefaultactions.h>
//  #include <qgsmessagebaritem.h>
#include <qgsproject.h>
// #include <qgsproviderregistry.h>
// #include <qgsrasterlayer.h>
#include <qgslayertree.h>
#include <qgsmapcanvassnappingutils.h>
#include <qgsmaptool.h>
#include <qgsmaptoolcapture.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerutils.h>

// START for attribute table
//  #include <qgsattributedialog.h>
//  #include <qgsattributetablemodel.h>
//  #include <qgsattributetableview.h>
//  #include <qgseditorwidgetfactory.h>
//  #include <qgsfeaturelistmodel.h>
//  #include <qgsfeaturelistview.h>
//  #include <qgsvectorlayercache.h>

// START for layer symbol
#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgslinesymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbollayer.h>
// #include <qgscategorizedsymbolrenderer.h>
// #include <qgsmapcanvas.h>
// #include <qgsmapoverviewcanvas.h>
// #include <qgsmarkersymbollayer.h>
// #include <qgsrendercontext.h>
// #include <qgssinglesymbolrenderer.h>
// #include <qgssymbol.h>
// #include <qgssymbollayer.h>
// #include <qgsvectorlayerrenderer.h>

// START for map tools
//  #include <qgsmaptool.h>
//  #include <qgsmaptoolidentify.h>
//  #include <qgsmaptoolpan.h>
//  #include <qgsmaptoolzoom.h>

// #include "bcmsloadform.h"
// #include "core/const.hpp"
#include "MapTools/qgsmaptooladdfeature.h"
#include "Widget/qgssnappingwidget.h"
#include "core/bcmsfeaturedef.h"
#include "core/qgsguivectorlayertools.h"
#include "maptools/qgsappmaptools.h"

BcmsApp *BcmsApp::sInstance = nullptr;

BcmsApp::BcmsApp(QWidget *parent) : QMainWindow(parent), ui(new Ui::BcmsApp) {
    if (sInstance) {
        QMessageBox::critical(
            this, tr("Multiple Instances of QgisApp"),
            tr("Multiple instances of QGIS application object "
               "detected.\nPlease contact the developers.\n"));
        abort();
    }
    sInstance = this;
    ui->setupUi(this);

    this->showMaximized();
    // showFullScreen();

    //! 布局
    QWidget *centralWidget = this->centralWidget();
    QGridLayout *centralLayout = new QGridLayout(centralWidget);
    centralWidget->setLayout(centralLayout);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    //! 初始化主畫布
    mMapCanvas = new QgsMapCanvas(centralWidget);
    mMapCanvas->setVisible(true);
    centralLayout->addWidget(mMapCanvas, 0, 0, 2, 1);
    // this->setCentralWidget(mMapCanvas);

    //! 初始化訊息列
    mMessageBar = new QgsMessageBar(centralWidget);
    mMessageBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    centralLayout->addWidget(mMessageBar, 0, 0, 1, 1);

    //! 初始化變更列表
    // mAreaTableView = new QTableView(this);
    ui->areasDock->setWindowTitle(QStringLiteral("未存檔變更"));
    // mLayerTreeView->setMessageBar(mMessageBar);
    // ui->areasDock->setWidget(mAreaTableView);

    //! 初始化圖層管理器
    mLayerTreeView = new QgsLayerTreeView(this);
    ui->layersDock->setWindowTitle(QStringLiteral("圖層管理器"));
    mLayerTreeView->setObjectName(QStringLiteral("m_layerTreeView"));
    initLayerTreeView();
    mLayerTreeView->setMessageBar(mMessageBar);
    ui->layersDock->setWidget(mLayerTreeView);

    //! 初始化地段載入選單
    mBcmsLoadForm = new BcmsLoadForm(this);
    mBcmsLoadForm->setFloating(true);
    mBcmsLoadForm->hide();

    //! 初始化 CAD 工具
    mAdvancedDigitizingDockWidget =
        new QgsAdvancedDigitizingDockWidget(mMapCanvas, this);
    mAdvancedDigitizingDockWidget->setWindowTitle(tr("Advanced Digitizing"));
    mAdvancedDigitizingDockWidget->setObjectName(
        QStringLiteral("AdvancedDigitizingTools"));

    //! 初始化矢量工具
    mVectorLayerTools = new QgsGuiVectorLayerTools();

    QShortcut *showAdvancedDigitizingDock =
        new QShortcut(QKeySequence(tr("Ctrl+4")), this);
    connect(showAdvancedDigitizingDock, &QShortcut::activated,
            mAdvancedDigitizingDockWidget, &QgsDockWidget::toggleUserVisible);
    showAdvancedDigitizingDock->setObjectName(
        QStringLiteral("ShowAdvancedDigitizingPanel"));
    showAdvancedDigitizingDock->setWhatsThis(
        tr("Show Advanced Digitizing Panel"));
    addDockWidget(Qt::LeftDockWidgetArea, mAdvancedDigitizingDockWidget);
    mAdvancedDigitizingDockWidget->show();

    //! 初始化工具列工具
    mMapTools = std::make_unique<QgsAppMapTools>(mMapCanvas,
                                                 mAdvancedDigitizingDockWidget);

    //! 初始化鎖點工具
    mSnappingDialog =
        new QgsSnappingWidget(QgsProject::instance(), mMapCanvas, this);
    connect(mSnappingDialog, &QgsSnappingWidget::snappingConfigChanged,
            QgsProject::instance(), [=] {
                QgsProject::instance()->setSnappingConfig(
                    mSnappingDialog->config());
            });
    QgsDockWidget *dock = new QgsDockWidget(
        tr("Snapping and Digitizing Options"), BcmsApp::instance());
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setWidget(mSnappingDialog);
    dock->setObjectName(QStringLiteral("Snapping and Digitizing Options"));
    addDockWidget(Qt::TopDockWidgetArea, dock);
    mSnappingDialogContainer = dock;
    mSnappingUtils = new QgsMapCanvasSnappingUtils(mMapCanvas, this);
    mMapCanvas->setSnappingUtils(mSnappingUtils);
    connect(QgsProject::instance(), &QgsProject::snappingConfigChanged,
            mSnappingUtils, &QgsSnappingUtils::setConfig);

    // QDialog *dialog = new QDialog(this, Qt::Tool);
    // dialog->setObjectName(QStringLiteral("snappingSettings"));
    // dialog->setWindowTitle(tr("Project Snapping Settings"));
    // QgsGui::enableAutoGeometryRestore(dialog);
    // QVBoxLayout *layout = new QVBoxLayout(dialog);
    // layout->addWidget(mSnappingDialog);
    // layout->setContentsMargins(0, 0, 0, 0);
    // mSnappingDialogContainer = dialog;
    // ui->tableView->setFixedWidth(300);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    mAreaListItemModel->setHorizontalHeaderLabels(
        {QStringLiteral("法定空地編號"), QStringLiteral("字串")});
    ui->tableView->setModel(mAreaListItemModel);

    //! 載入圖層
    initLayerDefine("./temp/400_0044_polygon.json");
    initLandDefine("./temp/400_0044_land.json");

    //! 連結
    connect(ui->actionToggle_Editing, SIGNAL(triggered()), this,
            SLOT(toggleEditing()));
    connect(ui->actionAdd_Feature, SIGNAL(triggered()), this,
            SLOT(addFeature()));
    connect(ui->actionAdd_Feature_2, SIGNAL(triggered()), this,
            SLOT(addFeature2()));
    connect(mLayerTreeView, SIGNAL(currentLayerChanged(QgsMapLayer *)), this,
            SLOT(changeSelectLayer(QgsMapLayer *)));
    connect(ui->actionSelect_Rectangle, SIGNAL(triggered()), this,
            SLOT(selectFeatures()));
    connect(ui->actionOpen_SectionForm, SIGNAL(triggered()), this,
            SLOT(openMBcmsLoadForm()));
    connect(ui->actionDelete_Selected, &QAction::triggered, this,
            [=] { deleteSelected(nullptr, nullptr, true); });
    connect(mBcmsLoadForm, &BcmsLoadForm::loadSignal, this, &BcmsApp::loadLand);
    connect(this, &BcmsApp::landLoaded, this, &BcmsApp::setLocate);
    connect(mLayerTreeView, SIGNAL(currentLayerChanged(QgsMapLayer *)), this,
            SLOT(updateToolBarState(QgsMapLayer *)));
    connect(mMapCanvas, SIGNAL(keyPressed(QKeyEvent *)), this,
            SLOT(keyPressEvent(QKeyEvent *)));

#ifdef DEVELOPING
    connect(ui->actionTest1, SIGNAL(triggered()), this, SLOT(testFunc()));
#else
    ui->actionTest1->setVisible(false);
#endif

    mMessageBar->pushMessage(QStringLiteral("成功載入地圖！"), tr(""),
                             Qgis::MessageLevel::Info);
}

BcmsApp::~BcmsApp() { delete ui; }

void BcmsApp::updateToolBarState(QgsMapLayer *layer) {
    // 更新編輯按鈕狀態
    if (layer->isEditable()) {
        ui->actionToggle_Editing->setChecked(true);
        mMapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::AddFeature));
    } else {
        ui->actionToggle_Editing->setChecked(false);
        mMapCanvas->setMapTool(
            mMapTools->mapTool(QgsAppMapTools::SelectFeatures));
    }
}

void BcmsApp::loadLand(const ILandCode &landCode) {
    //@ qDebug() << "landmon: " << landCode.landmon;
    // qDebug() << "landchild: " << landCode.landchild;
    // qDebug() << "zon: " << landCode.zon;
    // qDebug() << "section: " << landCode.section;
    try {
        // 建立 QNetworkAccessManager 對象
        QNetworkAccessManager manager;

        // 要發送的 API URL
        QString apiUrl = CONST::instance()->getGeoViewerJSServiceUrl() +
                         "/api/v3/land-data/get/geojson/" + landCode.zon + "/" +
                         landCode.section;

        // 發送 API 請求，取回 data 放到 resData
        QNetworkReply *res = manager.get(QNetworkRequest(apiUrl));
        QEventLoop event;
        connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
        event.exec();
        QByteArray resData = res->readAll();

        // 做檔案
        QString filename = "./temp/" + landCode.zon + "_" + landCode.section +
                           "_" + "land.json";
        QFile file(filename);

        // 寫入檔案
        if (file.open(QIODevice::WriteOnly)) {
            file.write(resData);
            file.close();
            qDebug() << "Response data written to file: " << filename;
        } else {
            qDebug() << "Failed to write response data to file: " << filename;
        }

        // 清理
        resetMap();

        // 載入建築物套繪資料
        loadBuilding(landCode);

        // 載入地圖
        initLandDefine(filename);

        // 異步發送地段載入訊號，用於定位到地號
        QTimer::singleShot(100, this,
                           [this, landCode]() { emit landLoaded(landCode); });

        // 最後修改程式名稱為加上地區地段
        this->setWindowTitle("BcmsApp " + landCode.zonDesc + "-" +
                             landCode.sectionDesc);
    } catch (const std::exception &e) {
        qWarning() << e.what();
    }
}

void BcmsApp::snappingOptions() { mSnappingDialogContainer->show(); }

void BcmsApp::loadBuilding(const ILandCode &landCode) {
    // 建立 QNetworkAccessManager 對象
    QNetworkAccessManager manager;

    // 要發送的 API URL
    QString apiUrl = CONST::instance()->getGeoViewerJSServiceUrl() +
                     "/api/v3/building-data/get/geojson/" + landCode.zon + "/" +
                     landCode.section;

    // 發送 API 請求，取回 data 放到 resData
    QNetworkReply *res = manager.get(QNetworkRequest(apiUrl));
    QEventLoop event;
    connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    QByteArray resData = res->readAll();

    // 做檔案
    QString filename = "./temp/" + landCode.zon + "_" + landCode.section + "_" +
                       "building.json";
    QFile file(filename);

    // 寫入檔案
    if (file.open(QIODevice::WriteOnly)) {
        file.write(resData);
        file.close();
        qDebug() << "Response data written to file: " << filename;
    } else {
        qDebug() << "Failed to write response data to file: " << filename;
    }

    // 載入檔案
    initLayerDefine(filename);
    // addVectorLayers(filename, QStringLiteral("法定騎樓"), AL);
    // addVectorLayers(filename, QStringLiteral("建築物"), BU);
    // addVectorLayers(filename, QStringLiteral("其他"), OTHER);
}
void BcmsApp::setLocate(const ILandCode &landCode) {
#ifdef DEVELOPING
    qDebug() << "in setLocate";
#endif
    auto landLayer = findLayerByName(QStringLiteral("地籍"));

    // 构建查询表达式
    QString expression = QString("\"landmon\" = '%1' AND \"landchild\" = '%2'")
                             .arg(landCode.landmon)
                             .arg(landCode.landchild);
    qDebug() << "expression: " << expression;

    // 创建查询请求
    QgsFeatureRequest request;
    request.setFilterExpression(expression);

    // 要素集合
    QgsFeatureIterator it = landLayer->getFeatures(request);
    if (it.isValid()) {
        // 抓出第一個符合的要素
        QgsFeature feat;
        it.nextFeature(feat);

        // 获取要素的几何信息
        QgsGeometry geometry = feat.geometry();
        QgsRectangle boundingBox = geometry.boundingBox();
        qDebug() << "boundingBox: " << boundingBox.toString();
        boundingBox.scale(8);

        // 定位到要素的几何范围
        mMapCanvas->setExtent(boundingBox);
        mMapCanvas->setVisible(true);
        mMapCanvas->freeze(false);
        mMapCanvas->refresh();
    } else {
        qDebug() << QStringLiteral("定位失敗!!!: ");
        mMessageBar->pushMessage(QStringLiteral("定位失敗!!!"), tr(""),
                                 Qgis::MessageLevel::Warning);
    }

    mMessageBar->pushMessage(QStringLiteral("成功定位！"), tr(""),
                             Qgis::MessageLevel::Info);
#ifdef DEVELOPING
    qDebug() << "done setLocate";
#endif
}

QgsVectorLayer *BcmsApp::findLayerByName(const QString &name) {
    for (QgsMapLayer *layer : mMapCanvasLayers) {
        QgsVectorLayer *vectorLayer = dynamic_cast<QgsVectorLayer *>(layer);
        if (vectorLayer) {
            // 检查图层的 name 属性是否匹配
            if (vectorLayer->name() == name) {
                // qDebug() << vectorLayer->name();
                return vectorLayer;
            }
        }
    }

    return nullptr;
}

void BcmsApp::resetMap() {
    try {
        disconnect(mLayerTreeView, SIGNAL(currentLayerChanged(QgsMapLayer *)),
                   this, SLOT(updateToolBarState(QgsMapLayer *)));
        disconnect(mMapCanvas, SIGNAL(keyPressed(QKeyEvent *)), this,
                   SLOT(keyPressEvent(QKeyEvent *)));
        // QgsProject::instance()->removeAllMapLayers();
        QgsProject::instance()->clear();
        mMapCanvasLayers.clear();
        mMapCanvas->setTheme("");
        connect(mLayerTreeView, SIGNAL(currentLayerChanged(QgsMapLayer *)),
                this, SLOT(updateToolBarState(QgsMapLayer *)));
        connect(mMapCanvas, SIGNAL(keyPressed(QKeyEvent *)), this,
                SLOT(keyPressEvent(QKeyEvent *)));
    } catch (const std::exception &e) {
        qWarning() << e.what();
    }
}

void BcmsApp::loadLayerIntoMap() {
    QgsProject::instance()->clear();
    foreach (const auto ele, mMapCanvasLayers) {
        QgsProject::instance()->addMapLayer(ele);
    }
}

void BcmsApp::log() { qDebug() << tr("in"); }

void BcmsApp::openMBcmsLoadForm() { mBcmsLoadForm->show(); }

void BcmsApp::testFunc() {
    try {
        auto testLand = new ILandCode();
        testLand->zon = "400";
        testLand->section = "0022";
        testLand->landmon = "0001";
        testLand->landchild = "0000";

        emit landLoaded(*testLand);
    } catch (const std::exception &e) {
        qDebug() << "in testFunc catch!";
        qWarning() << e.what();
        messageBar()->pushMessage(tr("Failed"), Qgis::MessageLevel::Critical);
    }
}

void BcmsApp::deleteSelected(QgsMapLayer *layer, QWidget *,
                             bool checkFeaturesVisible) {
    if (!layer) {
        layer = mLayerTreeView->currentLayer();
    }

    qDebug() << tr("Delete from layer: ") << layer->name();

    if (!layer) {
        messageBar()->pushMessage(
            tr("No Layer Selected"),
            tr("To delete features, you must select a vector layer in the "
               "legend"),
            Qgis::MessageLevel::Info);
        return;
    }

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
    if (!vlayer) {
        messageBar()->pushMessage(
            tr("No Vector Layer Selected"),
            tr("Deleting features only works on vector layers"),
            Qgis::MessageLevel::Info);
        return;
    }

    if (!(vlayer->dataProvider()->capabilities() &
          QgsVectorDataProvider::DeleteFeatures)) {
        messageBar()->pushMessage(
            tr("Provider does not support deletion"),
            tr("Data provider does not support deleting features"),
            Qgis::MessageLevel::Info);
        return;
    }

    if (!vlayer->isEditable()) {
        messageBar()->pushMessage(
            tr("Layer not editable"),
            tr("The current layer is not editable. Choose 'Start editing' in "
               "the digitizing toolbar."),
            Qgis::MessageLevel::Info);
        return;
    }

    // validate selection
    const int numberOfSelectedFeatures = vlayer->selectedFeatureCount();
    if (numberOfSelectedFeatures == 0) {
        messageBar()->pushMessage(
            tr("No Features Selected"),
            tr("The current layer has no selected features"),
            Qgis::MessageLevel::Info);
        return;
    }

    // display an "outside of view" warning
    bool confirmationServed = false;
    if (checkFeaturesVisible) {
        QgsFeature feat;
        QgsFeatureIterator it =
            vlayer->getSelectedFeatures(QgsFeatureRequest().setNoAttributes());
        bool allFeaturesInView = true;
        QgsRectangle viewRect = mMapCanvas->mapSettings().mapToLayerCoordinates(
            vlayer, mMapCanvas->extent());

        while (it.nextFeature(feat)) {
            if (allFeaturesInView &&
                !viewRect.intersects(feat.geometry().boundingBox())) {
                allFeaturesInView = false;
                break;
            }
        }

        if (!allFeaturesInView) {
            // for extra safety to make sure we are not removing geometries by
            // accident
            QMessageBox warning(
                QMessageBox::Warning,
                tr("Delete %n feature(s) from layer \"%1\"", nullptr,
                   numberOfSelectedFeatures)
                    .arg(vlayer->name()),
                tr("Some of the <b>%n</b> selected feature(s) about to be "
                   "deleted <b>are outside of the current map view</b>. Would "
                   "you still like to continue?",
                   nullptr, numberOfSelectedFeatures),
                QMessageBox::Yes | QMessageBox::Cancel, mMapCanvas);
            warning.button(QMessageBox::Yes)
                ->setText(tr("Delete %n Feature(s)", nullptr,
                             numberOfSelectedFeatures));
            int res = warning.exec();
            if (res != QMessageBox::Yes) return;
            confirmationServed = true;
        }
    }

    QgsVectorLayerUtils::QgsDuplicateFeatureContext infoContext;
    if (QgsVectorLayerUtils::impactsCascadeFeatures(
            vlayer, vlayer->selectedFeatureIds(), QgsProject::instance(),
            infoContext, QgsVectorLayerUtils::IgnoreAuxiliaryLayers)) {
        QString childrenInfo;
        int childrenCount = 0;
        const auto infoContextLayers = infoContext.layers();
        for (QgsVectorLayer *chl : infoContextLayers) {
            childrenCount += infoContext.duplicatedFeatures(chl).size();
            childrenInfo += (tr("%n feature(s) on layer \"%1\", ", nullptr,
                                infoContext.duplicatedFeatures(chl).size())
                                 .arg(chl->name()));
        }

        // for extra safety to make sure we know that the delete can have impact
        // on children and joins
        QMessageBox question(
            QMessageBox::Question,
            tr("Delete at least %1 feature(s) on other layer(s)")
                .arg(childrenCount),
            tr("Delete %1 feature(s) on layer \"%2\", %3 as well\nand all of "
               "its other descendants.\nDelete these features?")
                .arg(numberOfSelectedFeatures)
                .arg(vlayer->name(), childrenInfo),
            QMessageBox::Yes | QMessageBox::Cancel, mMapCanvas);
        question.button(QMessageBox::Yes)
            ->setText(
                tr("Delete %n Feature(s)", nullptr, numberOfSelectedFeatures));
        int res = question.exec();
        if (res != QMessageBox::Yes) return;
        confirmationServed = true;
    }

    if (!confirmationServed) {
        QgsSettings settings;
        const bool showConfirmation =
            settings
                .value(QStringLiteral("askToDeleteFeatures"), true,
                       QgsSettings::App)
                .toBool();
        if (showConfirmation) {
            QMessageBox confirmMessage(
                QMessageBox::Question,
                tr("Delete %n feature(s) from layer \"%1\"", nullptr,
                   numberOfSelectedFeatures)
                    .arg(vlayer->name()),
                tr("<b>%n</b> selected feature(s) is/are about to be deleted. "
                   "Would you like to continue?",
                   nullptr, numberOfSelectedFeatures),
                QMessageBox::Yes | QMessageBox::Cancel, mMapCanvas);
            confirmMessage.button(QMessageBox::Yes)
                ->setText(tr("Delete %n Feature(s)", nullptr,
                             numberOfSelectedFeatures));
            confirmMessage.setCheckBox(
                new QCheckBox(tr("Don't show this message again")));
            confirmMessage.checkBox()->setChecked(false);
            int res = confirmMessage.exec();
            if (res != QMessageBox::Yes) return;

            if (confirmMessage.checkBox()->isChecked()) {
                settings.setValue(QStringLiteral("askToDeleteFeatures"), false,
                                  QgsSettings::App);
            }
        }
    }

    vlayer->beginEditCommand(tr("Features deleted"));
    int deletedCount = 0;
    QgsVectorLayer::DeleteContext context(true, QgsProject::instance());
    if (!vlayer->deleteSelectedFeatures(&deletedCount, &context)) {
        messageBar()->pushMessage(
            tr("Problem deleting features"),
            tr("A problem occurred during deletion from layer \"%1\". %n "
               "feature(s) not deleted.",
               nullptr, numberOfSelectedFeatures - deletedCount)
                .arg(vlayer->name()),
            Qgis::MessageLevel::Warning);
    } else {
        const QList<QgsVectorLayer *> contextLayers =
            context.handledLayers(false);
        // if it affects more than one non-auxiliary layer, print feedback for
        // all descendants
        if (contextLayers.size() > 1) {
            deletedCount = 0;
            QString feedbackMessage;
            for (QgsVectorLayer *contextLayer : contextLayers) {
                feedbackMessage +=
                    tr("%1 on layer %2. ")
                        .arg(context.handledFeatures(contextLayer).size())
                        .arg(contextLayer->name());
                deletedCount += context.handledFeatures(contextLayer).size();
            }
            messageBar()->pushMessage(
                tr("%n feature(s) deleted: %1", nullptr, deletedCount)
                    .arg(feedbackMessage),
                Qgis::MessageLevel::Success);
        }

        // showStatusMessage(tr("%n feature(s) deleted.",
        //                      "number of features deleted", deletedCount));
    }

    vlayer->endEditCommand();
}

void BcmsApp::addFeature() {
    //    QgsMapTool *tool = mMapTools->mapTool(QgsAppMapTools::AddFeature);
    //    tool->set
    // tool->setLayer(layer);
    // mCanvas->setMapTool(tool);
    // QgsMapTool *addFeatureTool =
    //     new QgsMapToolAddFeature(mMapCanvas, mAdvancedDigitizingDockWidget,
    //                              QgsMapToolCapture::CaptureNone);
    // mMapCanvas->setMapTool(addFeatureTool);
    mMapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::AddFeature));
}

void BcmsApp::addFeature2() {
    // 如果被按起來，就走儲存那段
    if (!ui->actionAdd_Feature_2->isChecked()) {
        commitAllLayers();
        mMessageBar->pushMessage(QStringLiteral("建築物新增成功！"), tr(""),
                                 Qgis::MessageLevel::Info);
        return;
    }

    auto areaLayer = findLayerByName(QStringLiteral("法定空地"));
    mMessageBar->pushMessage(QStringLiteral("請繪製法定空地範圍"), tr(""),
                             Qgis::MessageLevel::Info);
    mMapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::AddFeature));
    areaLayer->startEditing();
    mLayerTreeView->setCurrentLayer(areaLayer);

    connect(areaLayer, SIGNAL(featureAdded(QgsFeatureId)), this,
            SLOT(onAreaAdd(QgsFeatureId)));
}

void BcmsApp::onAreaAdd(QgsFeatureId fid) {
    auto areaLayer = findLayerByName(QStringLiteral("法定空地"));
    disconnect(areaLayer, SIGNAL(featureAdded(QgsFeatureId)), this,
               SLOT(onAreaAdd(QgsFeatureId)));

    // 創建一個 QMessageBox 對話框
    QMessageBox msgBox;
    msgBox.setText(QStringLiteral("確認新增法定空地範圍"));
    // msgBox.setInformativeText(QStringLiteral("法定空地範圍新增成功，請繪製其他圖層"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setButtonText(QMessageBox::Ok, QStringLiteral("確認"));
    msgBox.setButtonText(QMessageBox::Cancel, QStringLiteral("取消"));

    // 顯示對話框並等待使用者選擇
    int ret = msgBox.exec();

    // 根據使用者的選擇執行不同的操作
    if (ret == QMessageBox::Cancel) {
        areaLayer->deleteFeature(fid);
        areaLayer->commitChanges();
        updateToolBarState(areaLayer);
        mMapCanvas->setMapTool(
            mMapTools->mapTool(QgsAppMapTools::SelectFeatures));
        ui->actionAdd_Feature_2->setChecked(false);
        mMessageBar->pushMessage(QStringLiteral("取消新增建築物！"), tr(""),
                                 Qgis::MessageLevel::Info);
        return;
    }
    qDebug() << "add Area!";
    // areaLayer->commitChanges();
    mMessageBar->pushMessage(
        QStringLiteral("法定空地範圍新增成功，請繪製其他圖層"), tr(""),
        Qgis::MessageLevel::Info);

    // 取得新增的法定空地並寫入到未存檔變更中
    auto areaFeature = areaLayer->getFeature(fid);
    auto areaObject = new IAreaObject;
    areaObject->area = areaFeature;
    areaFeature.setAttribute(tr("area_key"), areaObject->areaKey);
    areaLayer->updateFeature(areaFeature);
    mAreaList.append(areaObject);

    auto m = mAreaListItemModel;
    QList<QStandardItem *> r;
    r.append(new QStandardItem(areaObject->areaKey));
    r.append(new QStandardItem(areaObject->area.geometry().asJson()));
    m->appendRow(r);
    ui->tableView->setModel(m);

    // 同意修改
    areaLayer->commitChanges();

    // 開始繪製其他圖層
    editAllLayers();
}

void BcmsApp::selectFeatures() {
    mMapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::SelectFeatures));
}

QgsAttributeEditorContext BcmsApp::createAttributeEditorContext() {
    QgsAttributeEditorContext context;
    context.setVectorLayerTools(vectorLayerTools());
    context.setMapCanvas(mapCanvas());
    context.setCadDockWidget(cadDockWidget());
    context.setMainMessageBar(messageBar());
    return context;
}

void BcmsApp::toggleEditing() {
    const QList<QgsMapLayer *> selectedLayers =
        mLayerTreeView->selectedLayers();
    if (selectedLayers.empty()) return;

    QgsVectorLayer *selectedLayer =
        dynamic_cast<QgsVectorLayer *>(selectedLayers[0]);

    mMessageBar->pushMessage(selectedLayer->name(), tr(""),
                             Qgis::MessageLevel::Info);

    if (ui->actionToggle_Editing->isChecked()) {
        selectedLayer->startEditing();
    } else {
        selectedLayer->commitChanges();
    }
}

void BcmsApp::editAllLayers() {
    for (QgsMapLayer *layer : mMapCanvasLayers) {
        QgsVectorLayer *vectorLayer = dynamic_cast<QgsVectorLayer *>(layer);
        if (vectorLayer && vectorLayer->name() != QStringLiteral("法定空地")) {
            vectorLayer->startEditing();
        }
    }
    auto l =
        dynamic_cast<QgsVectorLayer *>(mLayerTreeView->selectedLayers()[0]);
    updateToolBarState(l);
}

void BcmsApp::commitAllLayers() {
    for (QgsMapLayer *layer : mMapCanvasLayers) {
        QgsVectorLayer *vectorLayer = dynamic_cast<QgsVectorLayer *>(layer);
        if (vectorLayer && vectorLayer->name() != QStringLiteral("法定空地")) {
            vectorLayer->commitChanges();
        }
    }
    auto l =
        dynamic_cast<QgsVectorLayer *>(mLayerTreeView->selectedLayers()[0]);
    updateToolBarState(l);
}

void BcmsApp::autoSelectAddedLayer(QList<QgsMapLayer *> layers) {
    if (!layers.isEmpty()) {
        QgsLayerTreeLayer *nodeLayer =
            QgsProject::instance()->layerTreeRoot()->findLayer(layers[0]->id());

        if (!nodeLayer) return;

        QModelIndex index = mLayerTreeView->node2index(nodeLayer);
        mLayerTreeView->setCurrentIndex(index);
    }
}

bool BcmsApp::checkGeoJSON(const QString &filePath) {
    // 检查文件是否存在
    QFile file(filePath);
    if (!file.exists()) {
        mMessageBar->pushMessage(QStringLiteral("套繪檔案不存在"),
                                 Qgis::MessageLevel::Warning, 5);
        return false;
    }

    // 读取文件内容
    if (!file.open(QIODevice::ReadOnly)) {
        mMessageBar->pushMessage(QStringLiteral("套繪檔案唯獨"),
                                 Qgis::MessageLevel::Warning, 5);
        return false;
    }

    // 读取文件内容为 JSON
    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (jsonDoc.isNull()) {
        mMessageBar->pushMessage(QStringLiteral("套繪檔案格式錯誤"),
                                 Qgis::MessageLevel::Warning, 5);
        return false;
    }

    // 检查文件是否包含 features
    QJsonObject jsonObject = jsonDoc.object();
    if (!jsonObject.contains("features")) {
        mMessageBar->pushMessage(QStringLiteral("圖層沒有要素"),
                                 Qgis::MessageLevel::Warning, 5);
        return false;
    }

    QJsonArray featuresArray = jsonObject.value("features").toArray();
    if (featuresArray.isEmpty()) {
        mMessageBar->pushMessage(QStringLiteral("圖層沒有要素"),
                                 Qgis::MessageLevel::Warning, 5);
        return false;
    }
    return true;
}

void BcmsApp::initLandDefine(QString filePath) {
    // 例外退出
    if (!checkGeoJSON(filePath)) return;

    // 宣告地籍圖層
    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};
    QgsVectorLayer *layer = new QgsVectorLayer(
        filePath, BcmsFeaturedef::instance().getSymbolDesc("LD"), "ogr",
        layerOptions);

    // 圖層樣式設定
    layer->setRenderer(BcmsFeaturedef::instance().getSymbolRenderer("LD"));

    // 圖層文字設定
    QgsPalLayerSettings layerLabelSetting;
    layerLabelSetting.fieldName = layer->fields()[0].name();
    layerLabelSetting.drawLabels = true;
    layerLabelSetting.centroidWhole = true;
    layer->setLabeling(new QgsVectorLayerSimpleLabeling(layerLabelSetting));
    layer->setLabelsEnabled(true);
    layer->updateFields();
    layer->setReadOnly(true);  // 地籍設定唯讀

    // 加入圖層
    QgsProject::instance()->addMapLayer(layer);
    mMapCanvasLayers.append(layer);
    mMapCanvas->setExtent(layer->extent());
    mMapCanvas->setLayers(mMapCanvasLayers);
    mMapCanvas->setVisible(true);
    mMapCanvas->freeze(false);
    mMapCanvas->refresh();

#ifdef DEVELOPING
    qDebug() << "done initLand";
#endif
}

void BcmsApp::initLayerDefine(QString filePath) {
    // 例外退出
    if (!checkGeoJSON(filePath)) return;

    try {
        auto layers = BcmsFeaturedef::instance().getIdList();
        foreach (const QString &id, *layers) {
            // 宣告地籍圖層
            const QgsVectorLayer::LayerOptions layerOptions{
                QgsProject::instance()->transformContext()};
            QgsVectorLayer *layer = new QgsVectorLayer(
                filePath, BcmsFeaturedef::instance().getSymbolDesc(id), "ogr",
                layerOptions);

            // 圖層檢索
            layer->setSubsetString("\"layer\" = '" + id + "'");

            QgsPalLayerSettings layerLabelSetting;
            layerLabelSetting.fieldName = layer->fields().field("label").name();
            layerLabelSetting.drawLabels = true;
            layerLabelSetting.centroidWhole = true;
            layer->setLabeling(
                new QgsVectorLayerSimpleLabeling(layerLabelSetting));
            layer->setLabelsEnabled(true);

            // 圖層樣式設定
            layer->setRenderer(
                BcmsFeaturedef::instance().getSymbolRenderer(id));

            // 加入圖層
            QgsProject::instance()->addMapLayer(layer);
            mMapCanvasLayers.append(layer);
            mMapCanvas->setExtent(layer->extent());
            mMapCanvas->setLayers(mMapCanvasLayers);
            mMapCanvas->setVisible(true);
            mMapCanvas->freeze(false);
            mMapCanvas->refresh();
        }
    } catch (const std::exception &e) {
        qWarning() << e.what();
    }
}

void BcmsApp::addVectorLayers(QString filePath, QString DisplayName,
                              QString layerTypes) {
    // 例外退出
    if (!checkGeoJSON(filePath)) return;

    // layerOptions.
    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};

    // 1. import layer.
    QgsVectorLayer *layer =
        new QgsVectorLayer(filePath, DisplayName, "ogr", layerOptions);

    QgsSimpleFillSymbolLayer *landSymbolLayer = new QgsSimpleFillSymbolLayer();
    QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer();
    QgsFillSymbol *fillSymbol = new QgsFillSymbol();
    QgsPalLayerSettings layerLabelSetting;
    QgsEditFormConfig formConfig = layer->editFormConfig();

    // 2. layer setting.
    if (layerTypes == "LD") {
        layer->setSubsetString("\"layer\" = 'LD'");
        // lineSymbolLayer->setColor(Qt::darkBlue);
        lineSymbolLayer->setColor(QColor("#0000ff"));
        lineSymbolLayer->setWidth(0.100000);
        fillSymbol->changeSymbolLayer(0, lineSymbolLayer);
        layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));

        layerLabelSetting.fieldName = layer->fields()[0].name();
        layerLabelSetting.drawLabels = true;
        layerLabelSetting.centroidWhole = true;
        layer->setLabeling(new QgsVectorLayerSimpleLabeling(layerLabelSetting));
        layer->setLabelsEnabled(true);
        layer->updateFields();

        layer->setReadOnly(true);
    } else if (layerTypes == "BU") {
        layer->setSubsetString("\"layer\" = 'BU'");

        landSymbolLayer->setColor(Qt::red);
        fillSymbol->changeSymbolLayer(0, landSymbolLayer);
        layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));

        layerLabelSetting.fieldName = layer->fields()[0].name();
        layerLabelSetting.drawLabels = true;
        layerLabelSetting.centroidWhole = true;
        layer->setLabeling(new QgsVectorLayerSimpleLabeling(layerLabelSetting));
        layer->setLabelsEnabled(true);

        // 設置屬性類型
        for (auto field : layer->fields()) {
            //@ qDebug() << field.displayName() << ", " <<
            // field.displayType()
            //          << ", " << field.type();
            if (field.displayName() == tr("layer")) {
                field.setDefaultValueDefinition(
                    QgsDefaultValue(QStringLiteral("\"BU\"")));
            }
        }
        layer->updateFields();

        // 設置禁用屬性
        formConfig.setReadOnly(2, true);  // 禁用第3個屬性欄位
        layer->setEditFormConfig(formConfig);

        mMapCanvas->setCurrentLayer(layer);
    } else if (layerTypes == "BA") {
        layer->setSubsetString("\"layer\" = 'BA'");

        landSymbolLayer->setColor(Qt::darkGreen);
        fillSymbol->changeSymbolLayer(0, landSymbolLayer);
        layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));
    } else if (layerTypes == "AL") {
        layer->setSubsetString("\"layer\" = 'AL'");

        landSymbolLayer->setColor(Qt::yellow);
        QgsSimpleFillSymbolLayer *temp = new QgsSimpleFillSymbolLayer();
        temp->setColor(QColor("#ff0000"));        // 設定填滿顏色
        temp->setBrushStyle(Qt::BDiagPattern);    // 設定渲染樣式
        temp->setStrokeColor(QColor("#232323"));  // 設定邊框顏色
        temp->setStrokeWidth(0.2);                // 設定邊框寬度
        temp->setStrokeStyle(Qt::SolidLine);      // 設定邊框渲染樣式

        fillSymbol->changeSymbolLayer(0, landSymbolLayer);
        fillSymbol->insertSymbolLayer(1, temp);
        layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));
    }

    QgsProject::instance()->addMapLayer(layer);
    mMapCanvasLayers.append(layer);
    mMapCanvas->setExtent(layer->extent());
    mMapCanvas->setLayers(mMapCanvasLayers);
    mMapCanvas->setVisible(true);
    mMapCanvas->freeze(false);
    mMapCanvas->refresh();
}

void BcmsApp::initLayerTreeView() {
    QgsLayerTreeModel *model =
        new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
    model->setFlag(QgsLayerTreeModel::AllowNodeRename);
    model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
    model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
    model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
    model->setAutoCollapseLegendNodes(10);
    mLayerTreeView->setModel(model);

    mLayerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(
        QgsProject::instance()->layerTreeRoot(), mMapCanvas, this);
    // connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)),
    //         mLayerTreeCanvasBridge, SLOT(writeProject(QDomDocument &)));
    // connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)),
    //         mLayerTreeCanvasBridge, SLOT(readProject(QDomDocument)));
    connect(QgsProject::instance()->layerTreeRegistryBridge(),
            SIGNAL(addedLayersToLayerTree(QList<QgsMapLayer *>)), this,
            SLOT(autoSelectAddedLayer(QList<QgsMapLayer *>)));
}

void BcmsApp::changeSelectLayer(QgsMapLayer *layer) {
    if (!layer) {
        return;
    }
    if (mMapCanvas) {
        mMapCanvas->setCurrentLayer(layer);
    }
}

void BcmsApp::keyPressEvent(QKeyEvent *event) {
    qDebug() << "key: " << event->key();
    if (event->key() == Qt::Key_A) {
        mAdvancedDigitizingDockWidget->setAngle(
            "90", QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut);
        qDebug() << "A key pressed";
    } else if (event->key() == Qt::Key_B) {
        qDebug() << "B key pressed";
        QgsSettings().setValue(QStringLiteral("/Cad/CommonAngle"), 90.0);
    } else if (event->key() == Qt::Key_N) {
        qDebug() << "N key pressed";
        QgsSettings().setValue(QStringLiteral("/Cad/CommonAngle"), 0.0);
    } else if (event->key() == Qt::Key_Delete) {
        qDebug() << "Delete key pressed";
        if (mLayerTreeView->selectedLayers().empty()) {
            QMainWindow::keyPressEvent(event);
            return;
        }
        auto l =
            dynamic_cast<QgsVectorLayer *>(mLayerTreeView->selectedLayers()[0]);
        if (l->selectedFeatureCount() > 0) {
            emit ui->actionDelete_Selected->trigger();
        }
    }

    // 调用基类的 keyPressEvent 以确保正確處理其他事件
    QMainWindow::keyPressEvent(event);
    event->accept();
}
