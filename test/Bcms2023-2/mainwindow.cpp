#include "mainwindow.h"

#include <qtextcodec.h>

#include <QDialog>
#include <QFileDialog>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMargins>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtDebug>
#include <cassert>

#include "qdockwidget.h"
#include "qgslayertree.h"
#include "ui_mainwindow.h"

// QGIS
#include <qgis.h>
#include <qgsabstractmaptoolhandler.h>
#include <qgsapplication.h>
#include <qgsattributeeditorcontext.h>
#include <qgslayertreegroup.h>
#include <qgslayertreelayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreeregistrybridge.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgsmaplayer.h>
#include <qgsmaptool.h>  // map 工具(編輯之類的)
#include <qgsmessagebar.h>
#include <qgsmessagelog.h>  // Qgis 訊息紀錄
#include <qgsmessageviewer.h>
#include <qgsproject.h>
#include <qgsrasterlayer.h>
#include <qgssettingsregistry.h>
#include <qgssettingsregistrycore.h>
#include <qgstransaction.h>       // Qgis 編輯交易模式相關
#include <qgstransactiongroup.h>  // Qgis 編輯交易模式相關
#include <qgsuserinputwidget.h>
#include <qgsvectorlayer.h>
#include <qshortcut.h>

#include "qgsguivectorlayertools.h"

// START layer label setting
#include <qgspallabeling.h>
#include <qgsvectorlayerlabeling.h>
// END layer label setting

// START layer symbol setting
#include <qgsadvanceddigitizingdockwidget.h>
#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgsmarkersymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>
// END layer symbol setting

#include "layertreeviewmenuprovider.h"
#include "maptools/qgsappmaptools.h"

// sub widget
#include "editmapdockwidget.h"

// 單例模式指向自己
MainWindow *MainWindow::sInstance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
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
    mAdvancedDigitizingDockWidget =
        new QgsAdvancedDigitizingDockWidget(mapCanvas, this);
    mAdvancedDigitizingDockWidget->setWindowTitle(tr("Advanced Digitizing"));
    mAdvancedDigitizingDockWidget->setObjectName(
        QStringLiteral("AdvancedDigitizingTools"));

    QShortcut *showAdvancedDigitizingDock =
        new QShortcut(QKeySequence(tr("Ctrl+4")), this);
    connect(showAdvancedDigitizingDock, &QShortcut::activated,
            mAdvancedDigitizingDockWidget, &QgsDockWidget::toggleUserVisible);
    showAdvancedDigitizingDock->setObjectName(
        QStringLiteral("ShowAdvancedDigitizingPanel"));
    showAdvancedDigitizingDock->setWhatsThis(
        tr("Show Advanced Digitizing Panel"));
    mMapTools = std::make_unique<QgsAppMapTools>(mapCanvas,
                                                 mAdvancedDigitizingDockWidget);
    addDockWidget(Qt::LeftDockWidgetArea, mAdvancedDigitizingDockWidget);
    mAdvancedDigitizingDockWidget->show();

    // User Input Dock Widget
    mUserInputDockWidget = new QgsUserInputWidget(mapCanvas);
    mUserInputDockWidget->setObjectName(QStringLiteral("UserInputDockWidget"));
    mUserInputDockWidget->setAnchorWidget(mapCanvas);
    mUserInputDockWidget->setAnchorWidgetPoint(QgsFloatingWidget::TopRight);
    mUserInputDockWidget->setAnchorPoint(QgsFloatingWidget::TopRight);

    // connections
    connect(ui->actionAdd_Vector, SIGNAL(triggered(bool)), this,
            SLOT(addVectorLayers()));
    connect(ui->actionPan_Map, SIGNAL(triggered()), this, SLOT(panMap()));
    connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(ui->actionZoom_To_Selected, SIGNAL(triggered()), this,
            SLOT(zoomToSelected()));
    connect(ui->actionSelect_Rectangle, SIGNAL(triggered()), this,
            SLOT(selectFeatures()));
    connect(ui->actionZoom_Full_Map, SIGNAL(triggered()), this,
            SLOT(zoomFull()));
    connect(ui->actionSelect_Edit_Area, SIGNAL(triggered()), this,
            SLOT(openEditMapDockWidget()));
    connect(ui->actionAdd_Feature, SIGNAL(triggered()), this,
            SLOT(addFeature()));
    connect(layerList, SIGNAL(currentLayerChanged(QgsMapLayer *)), this,
            SLOT(changeSelectLayer(QgsMapLayer *)));
    connect(layerList->layerTreeModel()->rootGroup(),
            &QgsLayerTreeNode::visibilityChanged, this, &MainWindow::markDirty);
    // connect(layerList->selectionModel(),
    // SIGNAL(QItemSelectionModel::selectionChanged()), this,
    // SLOT(changeSelectLayer()));

    //	connect(ui.actionAdd_Vector, SIGNAL( triggered()) ), this, SLOT(addV()
    //));
}

MainWindow::~MainWindow() { delete ui; }

QgsMessageBar *MainWindow::messageBar() {
    // Q_ASSERT( mInfoBar );
    return mInfoBar;
}

void MainWindow::toggleEditing() {
    const QList<QgsMapLayer *> layerList2 = layerList->selectedLayers();

    if (!layerList2.isEmpty()) {
        // if there are selected layers, try to toggle those.
        // mActionToggleEditing has already been triggered at this point so its
        // checked status has changed
        const bool shouldStartEditing = ui->actionToggle_Edit->isChecked();
        for (const auto layer : layerList2) {
            if (layer->supportsEditing() &&
                shouldStartEditing != layer->isEditable()) {
                toggleEditing(qobject_cast<QgsVectorLayer *>(layer), true);
            }
        }
    } else {
        // if there are no selected layers, try to toggle the current layer
        QgsMapLayer *currentLayer = activeLayer();
        if (currentLayer && currentLayer->supportsEditing()) {
            toggleEditing(qobject_cast<QgsVectorLayer *>(currentLayer), true);
        } else {
            // active although there's no layer active!?
            ui->actionToggle_Edit->setChecked(false);
            ui->actionToggle_Edit->setEnabled(false);
            // visibleMessageBar()->pushMessage(tr("Start editing failed"),
            //                                  tr("Layer cannot be edited"),
            //                                  Qgis::MessageLevel::Warning);
        }
    }
}

QgsMessageBar *MainWindow::visibleMessageBar() {
    // if (mDataSourceManagerDialog && mDataSourceManagerDialog->isVisible() &&
    //     mDataSourceManagerDialog->isModal()) {
    //     return mDataSourceManagerDialog->messageBar();
    // } else {
    return messageBar();
    // }
}

bool MainWindow::toggleEditing(QgsVectorLayer *vlayer, bool allowCancel) {
    if (!vlayer) {
        return false;
    }

    bool res = true;

    // Assume changes if:
    //  a) the layer reports modifications or
    //  b) its transaction group was modified
    QSet<QgsVectorLayer *> modifiedLayers;
    switch (QgsProject::instance()->transactionMode()) {
        case Qgis::TransactionMode::Disabled: {
            if (vlayer->isModified()) modifiedLayers.insert(vlayer);
        } break;
        case Qgis::TransactionMode::AutomaticGroups: {
            QString connString =
                QgsTransaction::connectionString(vlayer->source());
            QString key = vlayer->providerType();

            QMap<QPair<QString, QString>, QgsTransactionGroup *>
                transactionGroups = QgsProject::instance()->transactionGroups();
            QMap<QPair<QString, QString>, QgsTransactionGroup *>::iterator tIt =
                transactionGroups.find(qMakePair(key, connString));
            QgsTransactionGroup *tg =
                (tIt != transactionGroups.end() ? tIt.value() : nullptr);

            if (tg && tg->layers().contains(vlayer) && tg->modified()) {
                if (vlayer->isModified()) modifiedLayers.insert(vlayer);
                const QSet<QgsVectorLayer *> transactionGroupLayers =
                    tg->layers();
                for (QgsVectorLayer *iterLayer : transactionGroupLayers) {
                    if (iterLayer != vlayer && iterLayer->isModified())
                        modifiedLayers.insert(iterLayer);
                }
            }
        } break;
        case Qgis::TransactionMode::BufferedGroups:
            modifiedLayers =
                QgsProject::instance()->editBufferGroup()->modifiedLayers();
            break;
    }

    if (!vlayer->isEditable() && !vlayer->readOnly()) {
        if (!vlayer->supportsEditing()) {
            ui->actionToggle_Edit->setChecked(false);
            ui->actionToggle_Edit->setEnabled(false);
            // visibleMessageBar()->pushMessage(
            //     tr("Start editing failed"),
            //     tr("Provider cannot be opened for editing"),
            //     Qgis::MessageLevel::Warning);
            return false;
        }

        QgsProject::instance()->startEditing(vlayer);

        QString markerType =
            QgsSettingsRegistryCore::settingsDigitizingMarkerStyle.value();
        bool markSelectedOnly =
            QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected
                .value();

        // redraw only if markers will be drawn
        if ((!markSelectedOnly || vlayer->selectedFeatureCount() > 0) &&
            (markerType == QLatin1String("Cross") ||
             markerType == QLatin1String("SemiTransparentCircle"))) {
            vlayer->triggerRepaint();
        }
    } else if (modifiedLayers.size() > 0) {
        QMessageBox::StandardButtons buttons =
            QMessageBox::Save | QMessageBox::Discard;
        if (allowCancel) buttons |= QMessageBox::Cancel;

        QString modifiedLayerNames;
        if (modifiedLayers.size() == 1)
            modifiedLayerNames = (*modifiedLayers.constBegin())->name();
        else if (modifiedLayers.size() == 2)
            modifiedLayerNames =
                tr("%1 and %2")
                    .arg((*modifiedLayers.constBegin())->name(),
                         (*++modifiedLayers.constBegin())->name());
        else if (modifiedLayers.size() > 2)
            modifiedLayerNames =
                tr("%1, %2, …")
                    .arg((*modifiedLayers.constBegin())->name(),
                         (*++modifiedLayers.constBegin())->name());

        switch (QMessageBox::question(
            nullptr, tr("Stop Editing"),
            modifiedLayers.size() > 0
                ? tr("Do you want to save the changes to layers %1?")
                      .arg(modifiedLayerNames)
                : tr("Do you want to save the changes to layer %1?")
                      .arg(modifiedLayerNames),
            buttons)) {
            case QMessageBox::Cancel:
                res = false;
                break;

            case QMessageBox::Save: {
                QApplication::setOverrideCursor(Qt::WaitCursor);

                QStringList commitErrors;
                if (!QgsProject::instance()->commitChanges(commitErrors, true,
                                                           vlayer)) {
                    commitError(vlayer, commitErrors);
                    // Leave the in-memory editing state alone,
                    // to give the user a chance to enter different values
                    // and try the commit again later
                    res = false;
                }

                vlayer->triggerRepaint();

                QApplication::restoreOverrideCursor();
            } break;

            case QMessageBox::Discard: {
                QApplication::setOverrideCursor(Qt::WaitCursor);

                // QgsCanvasRefreshBlocker refreshBlocker;

                QStringList rollBackErrors;
                if (!QgsProject::instance()->rollBack(rollBackErrors, true,
                                                      vlayer)) {
                    visibleMessageBar()->pushMessage(
                        tr("Error"),
                        tr("Problems during roll back: '%1'")
                            .arg(rollBackErrors.join(" / ")),
                        Qgis::MessageLevel::Critical);
                    res = false;
                }

                vlayer->triggerRepaint();

                QApplication::restoreOverrideCursor();
                break;
            }

            default:
                break;
        }
    } else  // layer not modified
    {
        // QgsCanvasRefreshBlocker refreshBlocker;

        QStringList rollBackErrors;
        QgsProject::instance()->rollBack(rollBackErrors, true, vlayer);

        res = true;
        vlayer->triggerRepaint();
    }

    if (!res && vlayer == activeLayer()) {
        // while also called when layer sends editingStarted/editingStopped
        // signals, this ensures correct restoring of gui state if toggling was
        // canceled or layer commit/rollback functions failed
        // activateDeactivateLayerRelatedActions(vlayer);
    }

    return res;
}

QgsAttributeEditorContext MainWindow::createAttributeEditorContext() {
    QgsAttributeEditorContext context;
    context.setVectorLayerTools(vectorLayerTools());
    context.setMapCanvas(rMapCanvas());
    context.setCadDockWidget(cadDockWidget());
    context.setMainMessageBar(messageBar());
    return context;
}

void MainWindow::freezeCanvases(bool frozen) {
    const auto canvases = mapCanvases();
    for (QgsMapCanvas *canvas : canvases) {
        canvas->freeze(frozen);
    }
}

void MainWindow::updateLabelToolButtons() {
    bool enableMove = false, enableRotate = false, enablePin = false,
         enableShowHide = false, enableChange = false;

    QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
    for (QMap<QString, QgsMapLayer *>::iterator it = layers.begin();
         it != layers.end(); ++it) {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(it.value());
        if (vlayer && (vlayer->diagramsEnabled() || vlayer->labelsEnabled())) {
            enablePin = true;
            enableShowHide = true;
            enableMove = true;
            enableRotate = true;
            enableChange = true;

            break;
        }
    }

    //   mActionPinLabels->setEnabled( enablePin );
    //   mActionShowHideLabels->setEnabled( enableShowHide );
    //   mActionMoveLabel->setEnabled( enableMove );
    //   mActionRotateLabel->setEnabled( enableRotate );
    //   mActionChangeLabelProperties->setEnabled( enableChange );
}

// void MainWindow::switchToMapToolViaHandler() {
//     QAction *sourceAction = qobject_cast<QAction *>(sender());
//     if (!sourceAction) return;

//    QgsAbstractMapToolHandler *handler = nullptr;
//    for (QgsAbstractMapToolHandler *h : std::as_const(mMapToolHandlers)) {
//        if (h->action() == sourceAction) {
//            handler = h;
//            break;
//        }
//    }

//    if (!handler) return;

//    if (mapCanvas->mapTool() == handler->mapTool()) return;  // nothing to do

//    handler->setLayerForTool(activeLayer());
//    mapCanvas->setMapTool(handler->mapTool());
//}

// void MainWindow::registerMapToolHandler(QgsAbstractMapToolHandler *handler) {
//     if (!handler->action() || !handler->mapTool()) {
//         QgsMessageLog::logMessage(
//             tr("Map tool handler is not properly constructed"));
//         return;
//     }

//    mMapToolHandlers << handler;

//    // do setup work
//    handler->action()->setCheckable(true);
//    handler->mapTool()->setAction(handler->action());

//    connect(handler->action(), &QAction::triggered, this,
//            &MainWindow::switchToMapToolViaHandler);
//    // mMapToolGroup->addAction(handler->action());
//    QgsAbstractMapToolHandler::Context context;
//    handler->action()->setEnabled(
//        handler->isCompatibleWithLayer(activeLayer(), context));
//}

// void MainWindow::activateDeactivateLayerRelatedActions(QgsMapLayer *layer) {
//     updateLabelToolButtons();

//     updateLayerModifiedActions();

//     QgsAbstractMapToolHandler::Context context;
//     for (QgsAbstractMapToolHandler *handler :
//     std::as_const(mMapToolHandlers)) {
//         handler->action()->setEnabled(
//             handler->isCompatibleWithLayer(layer, context));
//         if (handler->mapTool() == mapCanvas->mapTool()) {
//             if (!handler->action()->isEnabled()) {
//                 mapCanvas->unsetMapTool(handler->mapTool());
//                 mActionPan->trigger();
//             } else {
//                 handler->setLayerForTool(layer);
//             }
//         }
//     }

//     bool identifyModeIsActiveLayer =
//         QgsSettings().enumValue(QStringLiteral("/Map/identifyMode"),
//                                 QgsMapToolIdentify::ActiveLayer) ==
//         QgsMapToolIdentify::ActiveLayer;

//     if (!layer) {
//         mMenuSelect->setEnabled(false);
//         mActionSelectFeatures->setEnabled(false);
//         mActionSelectPolygon->setEnabled(false);
//         mActionSelectFreehand->setEnabled(false);
//         mActionSelectRadius->setEnabled(false);
//         mActionIdentify->setEnabled(true);
//         mActionSelectByExpression->setEnabled(false);
//         mActionSelectByForm->setEnabled(false);
//         mActionLabeling->setEnabled(false);
//         mActionOpenTable->setEnabled(false);
//         mMenuFilterTable->setEnabled(false);
//         mActionOpenTableSelected->setEnabled(false);
//         mActionOpenTableVisible->setEnabled(false);
//         mActionOpenTableEdited->setEnabled(false);
//         mActionSelectAll->setEnabled(false);
//         mActionReselect->setEnabled(false);
//         mActionInvertSelection->setEnabled(false);
//         mActionOpenFieldCalc->setEnabled(false);
//         mActionToggleEditing->setEnabled(false);
//         mActionToggleEditing->setChecked(false);
//         mActionSaveLayerEdits->setEnabled(false);
//         mActionSaveLayerDefinition->setEnabled(false);
//         mActionLayerSaveAs->setEnabled(false);
//         mActionLayerProperties->setEnabled(false);
//         mActionLayerSubsetString->setEnabled(false);
//         mActionAddToOverview->setEnabled(false);
//         mActionFeatureAction->setEnabled(false);
//         mActionAddFeature->setEnabled(false);
//         mMenuEditGeometry->setEnabled(false);
//         mActionMoveFeature->setEnabled(false);
//         mActionMoveFeatureCopy->setEnabled(false);
//         mActionRotateFeature->setEnabled(false);
//         mActionScaleFeature->setEnabled(false);
//         mActionOffsetCurve->setEnabled(false);
//         mActionVertexTool->setEnabled(false);
//         mActionVertexToolActiveLayer->setEnabled(false);
//         mActionDeleteSelected->setEnabled(false);
//         mActionCutFeatures->setEnabled(false);
//         mActionCopyFeatures->setEnabled(false);
//         mActionPasteFeatures->setEnabled(false);
//         mActionCopyStyle->setEnabled(false);
//         mActionPasteStyle->setEnabled(false);
//         mActionCopyLayer->setEnabled(false);
//         // pasting should be allowed if there is a layer in the clipboard
//         mActionPasteLayer->setEnabled(
//             clipboard()->hasFormat(QStringLiteral(QGSCLIPBOARD_MAPLAYER_MIME)));
//         mActionReverseLine->setEnabled(false);
//         mActionTrimExtendFeature->setEnabled(false);

//         if (mUndoDock && mUndoDock->widget())
//             mUndoDock->widget()->setEnabled(false);
//         mActionUndo->setEnabled(false);
//         mActionRedo->setEnabled(false);
//         mActionSimplifyFeature->setEnabled(false);
//         mActionAddRing->setEnabled(false);
//         mActionFillRing->setEnabled(false);
//         mActionAddPart->setEnabled(false);
//         mActionDeleteRing->setEnabled(false);
//         mActionDeletePart->setEnabled(false);
//         mActionReshapeFeatures->setEnabled(false);
//         mActionSplitFeatures->setEnabled(false);
//         mActionSplitParts->setEnabled(false);
//         mActionMergeFeatures->setEnabled(false);
//         mMenuEditAttributes->setEnabled(false);
//         mActionMergeFeatureAttributes->setEnabled(false);
//         mActionMultiEditAttributes->setEnabled(false);
//         mActionRotatePointSymbols->setEnabled(false);
//         mActionOffsetPointSymbol->setEnabled(false);

//         mActionPinLabels->setEnabled(false);
//         mActionShowHideLabels->setEnabled(false);
//         mActionMoveLabel->setEnabled(false);
//         mActionRotateLabel->setEnabled(false);
//         mActionChangeLabelProperties->setEnabled(false);

//         mActionDiagramProperties->setEnabled(false);

//         mActionLocalHistogramStretch->setEnabled(false);
//         mActionFullHistogramStretch->setEnabled(false);
//         mActionLocalCumulativeCutStretch->setEnabled(false);
//         mActionFullCumulativeCutStretch->setEnabled(false);
//         mActionIncreaseBrightness->setEnabled(false);
//         mActionDecreaseBrightness->setEnabled(false);
//         mActionIncreaseContrast->setEnabled(false);
//         mActionDecreaseContrast->setEnabled(false);
//         mActionIncreaseGamma->setEnabled(false);
//         mActionDecreaseGamma->setEnabled(false);
//         mActionPanToSelected->setEnabled(false);
//         mActionZoomActualSize->setEnabled(false);
//         mActionZoomToSelected->setEnabled(false);
//         mActionZoomToLayers->setEnabled(false);
//         mActionZoomToLayer->setEnabled(false);

//         enableMeshEditingTools(false);
//         mDigitizingTechniqueManager->enableDigitizingTechniqueActions(false);

//         return;
//     }

QList<QgsMapCanvas *> MainWindow::mapCanvases() {
    // filter out browser canvases -- they are children of app, but a different
    // kind of beast, and here we only want the main canvas or dock canvases
    auto canvases = findChildren<QgsMapCanvas *>();
    canvases.erase(
        std::remove_if(canvases.begin(), canvases.end(),
                       [](QgsMapCanvas *canvas) {
                           return !canvas ||
                                  canvas->property("browser_canvas").toBool();
                       }),
        canvases.end());
    return canvases;
}

void MainWindow::commitError(QgsVectorLayer *vlayer,
                             const QStringList &commitErrorsList) {
    QStringList commitErrors = commitErrorsList;
    if (vlayer && commitErrors.isEmpty()) commitErrors = vlayer->commitErrors();

    if (vlayer && !vlayer->allowCommit() && commitErrors.empty()) {
        return;
    }

    const QString messageText =
        vlayer ? tr("Could not commit changes to layer %1").arg(vlayer->name())
               : tr("Could not commit changes");

    QgsMessageViewer *mv = new QgsMessageViewer();
    mv->setWindowTitle(tr("Commit Errors"));
    mv->setMessageAsPlainText(
        messageText + "\n\n" +
        tr("Errors: %1\n").arg(commitErrors.join(QLatin1String("\n  "))));

    QToolButton *showMore = new QToolButton();
    QAction *act = new QAction(showMore);
    act->setText(tr("Show more"));
    showMore->setStyleSheet(
        QStringLiteral("background-color: rgba(255, 255, 255, 0); color: "
                       "black; text-decoration: underline;"));
    showMore->setCursor(Qt::PointingHandCursor);
    showMore->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    showMore->addAction(act);
    showMore->setDefaultAction(act);
    connect(showMore, &QToolButton::triggered, mv, &QDialog::exec);
    connect(showMore, &QToolButton::triggered, showMore, &QObject::deleteLater);

    // no timeout set, since notice needs attention and is only shown first
    // time layer is labeled
    QgsMessageBarItem *errorMsg =
        new QgsMessageBarItem(tr("Commit errors"), messageText, showMore,
                              Qgis::MessageLevel::Warning, 0, messageBar());
    messageBar()->pushItem(errorMsg);
}

void MainWindow::openEditMapDockWidget() {
    QDockWidget *dock = new EditMapDockWidget();
    Qt::DockWidgetArea area = Qt::NoDockWidgetArea;
    dock->setFloating(true);
    addDockWidget(area, dock);
}

void MainWindow::zoomFull() { mapCanvas->zoomToProjectExtent(); }

void MainWindow::markDirty() {
    qDebug() << "linyejoe2: ";
    // notify the project that there was a change
    QgsProject::instance()->setDirty(true);
    // hideDeselectedLayers();
}

// void QgisApp::hideAllLayers()
// {
//   QgsDebugMsgLevel( QStringLiteral( "hiding all layers!" ), 3 );

//   const auto constChildren =
//   mLayerTreeView->layerTreeModel()->rootGroup()->children(); for (
//   QgsLayerTreeNode *node : constChildren )
//   {
//     node->setItemVisibilityCheckedRecursive( false );
//   }
// }

// void QgisApp::showAllLayers()
// {
//   QgsDebugMsgLevel( QStringLiteral( "Showing all layers!" ), 3 );
//   mLayerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive(
//   true );
// }

void MainWindow::hideDeselectedLayers() {
    QList<QgsLayerTreeLayer *> selectedLayerNodes =
        layerList->selectedLayerNodes();

    const auto constFindLayers =
        layerList->layerTreeModel()->rootGroup()->findLayers();
    for (QgsLayerTreeLayer *nodeLayer : constFindLayers) {
        if (selectedLayerNodes.contains(nodeLayer)) continue;
        nodeLayer->setItemVisibilityChecked(false);
    }
}

void MainWindow::changeSelectLayer(QgsMapLayer *layer) {
    if (!layer) {
        return;
    }
    if (mapCanvas) {
        mapCanvas->setCurrentLayer(layer);
    }
    // const QList<QgsLayerTreeLayer *> selectedLayers = layerList ?
    // layerList->selectedLayerNodes() : QList<QgsLayerTreeLayer *>();

    // qDebug() << "linyejoe1: " << selectedLayers;
    // qDebug() << "linyejoe2: " << layer;

    // mapCanvas->setCurrentLayer(layer);
}

QgsMapLayer *MainWindow::activeLayer() {
    return layerList ? layerList->currentLayer() : nullptr;
}

void MainWindow::addFeature() {
    mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::AddFeature));
}

void MainWindow::selectFeatures() {
    mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::SelectFeatures));
}

void MainWindow::panMap() {
    mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::Pan));
}

void MainWindow::zoomIn() {
    mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::ZoomIn));
}

void MainWindow::zoomOut() {
    mapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::ZoomOut));
}

void MainWindow::zoomToSelected() {
    const QList<QgsMapLayer *> layers = layerList->selectedLayers();

    if (layers.size() > 1)
        mapCanvas->zoomToSelected(layers);
    else
        mapCanvas->zoomToSelected();
}

QgsMapCanvas *MainWindow::rMapCanvas() {
    Q_ASSERT(mapCanvas);
    return mapCanvas;
}

void MainWindow::addUserInputWidget(QWidget *widget) {
    mUserInputDockWidget->addUserInputWidget(widget);
}

void MainWindow::setDemo() {
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定空地"),
                    BA);
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("法定騎樓"),
                    AL);
    addVectorLayers("./temp/400_0044_polygon.json", QStringLiteral("建築物"),
                    BU);

    // layerOptions.
    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};

    // 1. import land.
    QgsVectorLayer *landLayer =
        new QgsVectorLayer("./temp/400_0044_land.json", QStringLiteral("地籍"),
                           "ogr", layerOptions);

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

    landLayer->setLabeling(
        new QgsVectorLayerSimpleLabeling(landLayerLabelSetting));
    landLayer->setLabelsEnabled(true);
    // END of set land label

    QgsProject::instance()->addMapLayer(landLayer);
    mapCanvasLayerSet.append(landLayer);
    mapCanvas->setExtent(landLayer->extent());
    mapCanvas->setLayers(mapCanvasLayerSet);
    mapCanvas->setVisible(true);
    mapCanvas->freeze(false);
    mapCanvas->refresh();
}

void MainWindow::addVectorLayers(QString filePath, QString DisplayName,
                                 LayerTypes layerTypes) {
    // layerOptions.
    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};

    // 1. import layer.
    QgsVectorLayer *layer =
        new QgsVectorLayer(filePath, DisplayName, "ogr", layerOptions);

    QgsSimpleFillSymbolLayer *landSymbolLayer = new QgsSimpleFillSymbolLayer();
    QgsFillSymbol *fillSymbol = new QgsFillSymbol();
    QgsPalLayerSettings layerLabelSetting;

    // 2. layer setting.
    switch (layerTypes) {
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

void MainWindow::addVectorLayers() {
    QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "",
                                                    "*.shp, *.json");
    QStringList temp = filename.split(QDir::separator());
    QString basename = temp.at(temp.size() - 1);
    // auto layerOptions = new QgsVectorLayer::LayerOptions(true, false);

    const QgsVectorLayer::LayerOptions layerOptions{
        QgsProject::instance()->transformContext()};
    QgsVectorLayer *vecLayer =
        new QgsVectorLayer(filename, basename, "ogr", layerOptions);
    if (!vecLayer->isValid()) {
        QMessageBox::critical(this, "error", "layer is invalid");
        return;
    }

    auto list = vecLayer->attributeList();
    QgsPalLayerSettings layerSetting;
    // layerSetting.drawLabels = true;
    // // set show label or not. layerSetting.fieldName =
    // vecLayer->fields()[1].name();
    // // set show witch property. layerSetting.centroidWhole = true;
    // // set label show at center

    // vecLayer->setLabeling(new
    // QgsVectorLayerSimpleLabeling(layerSetting));
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

void MainWindow::initLayerTreeView() {
    QgsLayerTreeModel *model =
        new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
    model->setFlag(QgsLayerTreeModel::AllowNodeRename);
    model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
    model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
    model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
    model->setAutoCollapseLegendNodes(10);
    layerList->setModel(model);

    // 添加右键菜单
    layerList->setMenuProvider(
        new LayerTreeViewMenuProvider(layerList, mapCanvas));

    connect(QgsProject::instance()->layerTreeRegistryBridge(),
            SIGNAL(addedLayersToLayerTree(QList<QgsMapLayer *>)), this,
            SLOT(autoSelectAddedLayer(QList<QgsMapLayer *>)));

    //	// 设置这个路径是为了获取图标文件
    //	QString iconDir = "../images/themes/default/";

    //	// add group tool button
    //	QToolButton *btnAddGroup = new QToolButton();
    //	btnAddGroup->setAutoRaise(true);
    //	btnAddGroup->setIcon(QIcon(iconDir + "mActionAdd.png"));

    //	btnAddGroup->setToolTip(tr("Add Group"));
    //	connect(btnAddGroup, SIGNAL(clicked()),
    // layerList->defaultActions(),
    // SLOT(addGroup()));

    //	// expand / collapse tool buttons
    //	QToolButton *btnExpandAll = new QToolButton();
    //	btnExpandAll->setAutoRaise(true);
    //	btnExpandAll->setIcon(QIcon(iconDir + "mActionExpandTree.png"));
    //	btnExpandAll->setToolTip(tr("Expand All"));
    //	connect(btnExpandAll, SIGNAL(clicked()), layerList,
    // SLOT(expandAll()));

    //	QToolButton *btnCollapseAll = new QToolButton();
    //	btnCollapseAll->setAutoRaise(true);
    //	btnCollapseAll->setIcon(QIcon(iconDir +
    //"mActionCollapseTree.png")); 	btnCollapseAll->setToolTip(tr("Collapse
    // All")); 	connect(btnCollapseAll, SIGNAL(clicked()), layerList,
    // SLOT(collapseAll()));

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
    //	layerListDock->setAllowedAreas(Qt::LeftDockWidgetArea |
    // Qt::RightDockWidgetArea);

    //	QWidget *w = new QWidget();
    //	w->setLayout(vboxLayout);
    //	layerListDock->setWidget(w);
    //	addDockWidget(Qt::RightDockWidgetArea, layerListDock);

    //	// 連接畫布和圖層列表
    layerListBridge = new QgsLayerTreeMapCanvasBridge(
        QgsProject::instance()->layerTreeRoot(), mapCanvas, this);
    connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)),
            layerListBridge, SLOT(writeProject(QDomDocument &)));
    connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)),
            layerListBridge, SLOT(readProject(QDomDocument)));
}

void MainWindow::autoSelectAddedLayer(QList<QgsMapLayer *> layers) {
    if (layers.count()) {
        QgsLayerTreeLayer *nodeLayer =
            QgsProject::instance()->layerTreeRoot()->findLayer(layers[0]->id());

        if (!nodeLayer) {
            return;
        }

        QModelIndex index = layerList->layerTreeModel()->node2index(nodeLayer);
        layerList->setCurrentIndex(index);
    }
}

void MainWindow::addDockWidget(Qt::DockWidgetArea area,
                               QDockWidget *dockwidget) {
    QMainWindow::addDockWidget(area, dockwidget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    dockwidget->show();
    mapCanvas->refresh();
}
