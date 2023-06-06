#include "bcmsapp.h"

// #include "bcmsloadform.h"
// #include "core/const.hpp"
#include "core/qgsguivectorlayertools.h"
#include "maptools/qgsappmaptools.h"
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
#include <qgsmaptool.h>
#include <qgsmaptoolcapture.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerutils.h>

#include "MapTools/qgsmaptooladdfeature.h"

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

    //! 載入圖層
    addVectorLayers("./temp/400_0044_land.json", QStringLiteral("地籍"), LAND);
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定空地"),
                    BA);
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定騎樓"),
                    AL);
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("建築物"),
                    BU);

    //! 連結
    connect(ui->actionToggle_Editing, SIGNAL(triggered()), this,
            SLOT(toggleEditing()));
    connect(ui->actionAdd_Feature, SIGNAL(triggered()), this,
            SLOT(addFeature()));
    connect(mLayerTreeView, SIGNAL(currentLayerChanged(QgsMapLayer *)), this,
            SLOT(changeSelectLayer(QgsMapLayer *)));
    connect(ui->actionSelect_Rectangle, SIGNAL(triggered()), this,
            SLOT(selectFeatures()));
    connect(ui->actionOpen_SectionForm, SIGNAL(triggered()), this,
            SLOT(openMBcmsLoadForm()));
    connect(ui->actionDelete_Selected, &QAction::triggered, this,
            [=] { deleteSelected(nullptr, nullptr, true); });
    connect(mBcmsLoadForm, &BcmsLoadForm::loadSignal, this, &BcmsApp::loadLand);

    mMessageBar->pushMessage(QStringLiteral("成功載入地圖！"), tr(""),
                             Qgis::MessageLevel::Info);
}

BcmsApp::~BcmsApp() { delete ui; }

void BcmsApp::loadLand(const ILandCode &landCode) {
    //@ qDebug() << "landmon: " << landCode.landmon;
    // qDebug() << "landchild: " << landCode.landchild;
    // qDebug() << "zon: " << landCode.zon;
    // qDebug() << "section: " << landCode.section;

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
    QString filename =
        "./temp/" + landCode.zon + "_" + landCode.section + "_" + "land.json";
    QFile file(filename);

    // 寫入檔案
    if (file.open(QIODevice::WriteOnly)) {
        file.write(resData);
        file.close();
        qDebug() << "Response data written to file: " << filename;
    } else {
        qDebug() << "Failed to write response data to file: " << filename;
    }

    // 清理後載入地圖
    resetMap();
    addVectorLayers(filename, QStringLiteral("地籍"), LAND);

    // 最後修改程式名稱為加上地區地段
    this->setWindowTitle("BcmsApp " + landCode.zonDesc + "-" +
                         landCode.sectionDesc);

    // 載入建築物套繪資料
    loadBuilding(landCode);
}

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
    addVectorLayers(filename, QStringLiteral("法定空地"), BA);
    addVectorLayers(filename, QStringLiteral("法定騎樓"), AL);
    addVectorLayers(filename, QStringLiteral("建築物"), BU);
    // addVectorLayers(filename, QStringLiteral("其他"), OTHER);

    // 定位到該去的地號
    // setLocate(landCode);
}
void BcmsApp::setLocate(const ILandCode &landCode) {}

void BcmsApp::resetMap() {
    QgsProject::instance()->clear();
    mMapCanvasLayers.clear();
    mMapCanvas->setTheme("");
}

void BcmsApp::log() { qDebug() << tr("in"); }

void BcmsApp::openMBcmsLoadForm() { mBcmsLoadForm->show(); }

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

void BcmsApp::addVectorLayers(QString filePath, QString DisplayName,
                              LayerTypes layerTypes) {
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
    switch (layerTypes) {
        case LAND:
            layer->setSubsetString("\"layer\" = 'LAND'");
            lineSymbolLayer->setColor(Qt::darkBlue);
            lineSymbolLayer->setWidth(0.3);
            fillSymbol->changeSymbolLayer(0, lineSymbolLayer);
            layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));

            layerLabelSetting.fieldName = layer->fields()[0].name();
            layerLabelSetting.drawLabels = true;
            layerLabelSetting.centroidWhole = true;
            layer->setLabeling(
                new QgsVectorLayerSimpleLabeling(layerLabelSetting));
            layer->setLabelsEnabled(true);
            layer->updateFields();

            layer->setReadOnly(true);
            break;
        case BU:
            layer->setSubsetString("\"layer\" = 'BU'");

            landSymbolLayer->setColor(Qt::red);
            fillSymbol->changeSymbolLayer(0, landSymbolLayer);
            layer->setRenderer(new QgsSingleSymbolRenderer(fillSymbol));

            layerLabelSetting.fieldName = layer->fields()[0].name();
            layerLabelSetting.drawLabels = true;
            layerLabelSetting.centroidWhole = true;
            layer->setLabeling(
                new QgsVectorLayerSimpleLabeling(layerLabelSetting));
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
        case OTHER:
        // TODO
        // layer->setSubsetString("\"layer\" = 'OTHER'");
        default:
            break;
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
    connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)),
            mLayerTreeCanvasBridge, SLOT(writeProject(QDomDocument &)));
    connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)),
            mLayerTreeCanvasBridge, SLOT(readProject(QDomDocument)));
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
