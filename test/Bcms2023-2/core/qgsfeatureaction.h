﻿#ifndef QGSFEATUREACTION_H
#define QGSFEATUREACTION_H

#include <qgsfeature.h>
#include <qgshighlight.h>

#include <QAction>
#include <QList>
#include <QPair>
#include <QPointer>
#include <QUuid>

// #include "mainwindow.h"

class QgsIdentifyResultsDialog;
class QgsVectorLayer;
class QgsAttributeDialog;
class QgsExpressionContextScope;

class QgsFeatureAction : public QAction {
    Q_OBJECT

   public:
    QgsFeatureAction(const QString &name, QgsFeature &f, QgsVectorLayer *vl,
                     QUuid actionId = QUuid(), int defaultAttr = -1,
                     QObject *parent = nullptr);

    enum class AddFeatureResult {
        Success = 0,
        LayerStateError = 1,
        Canceled = 2,
        Pending = 3,
        FeatureError = 4
    };

    /**
     * Add a new feature to the layer.
     * Will set the default values to recently used or provider defaults based
     * on settings and override with values in defaultAttributes if provided.
     *
     * \param defaultAttributes  Provide some default attributes here if
     * desired. \param scope              Scope of the expression \param
     * showModal          If the used dialog should be modal or not \param
     * hideParent         If the parent widget should be hidden, when the used
     * dialog is opened \param highlight          Optional canvas highlight for
     * feature
     *
     * \returns result if feature was added if showModal is true. If showModal
     * is FALSE, returns Pending in every case
     */
    AddFeatureResult addFeature(
        const QgsAttributeMap &defaultAttributes = QgsAttributeMap(),
        bool showModal = true,
        std::unique_ptr<QgsExpressionContextScope> scope =
            std::unique_ptr<QgsExpressionContextScope>(),
        bool hideParent = false,
        std::unique_ptr<QgsHighlight> highlight =
            std::unique_ptr<QgsHighlight>());

   public slots:
    void execute();
    bool viewFeatureForm(QgsHighlight *h = nullptr);
    bool editFeature(bool showModal = true);

    /**
     * Sets whether to force suppression of the attribute form popup after
     * creating a new feature. If \a force is TRUE, then regardless of any user
     * settings, form settings, etc, the attribute form will ALWAYS be
     * suppressed.
     */
    void setForceSuppressFormPopup(bool force);

    /**
     * Returns the added feature or invalid feature in case addFeature() was not
     * successful.
     */
    QgsFeature feature() const;

   signals:

    /**
     * This signal is emitted when the add feature process is finished.
     * Either during the call to addFeature() already or when the dialog is
     * eventually closed (accepted or canceled).
     *
     * \since QGIS 3.8
     */
    void addFeatureFinished();

   private slots:
    void onFeatureSaved(const QgsFeature &feature);
    void unhideParentWidget();
    void hideParentWidget();

   private:
    QgsAttributeDialog *newDialog(bool cloneFeature);

    QgsVectorLayer *mLayer = nullptr;
    QgsFeature *mFeature = nullptr;
    QUuid mActionId;
    int mIdx;

    bool mFeatureSaved;

    bool mForceSuppressFormPopup = false;
};

#endif
