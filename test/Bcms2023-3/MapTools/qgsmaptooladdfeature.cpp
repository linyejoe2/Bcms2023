#include "qgsmaptooladdfeature.h"

#include <qgsadvanceddigitizingdockwidget.h>
#include <qgsapplication.h>
#include <qgsattributedialog.h>
#include <qgscurvepolygon.h>
#include <qgsexception.h>
#include <qgsexpressioncontextutils.h>
#include <qgsfields.h>
#include <qgsgeometry.h>
#include <qgshighlight.h>
#include <qgslinestring.h>
#include <qgslogger.h>
#include <qgsmapcanvas.h>
#include <qgsmapmouseevent.h>
#include <qgsmultipoint.h>
#include <qgspolygon.h>
#include <qgsproject.h>
#include <qgsrubberband.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include <QSettings>

#include "qgsfeatureaction.h"

QgsMapToolAddFeature::QgsMapToolAddFeature(
    QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget,
    CaptureMode mode)
    : QgsMapToolDigitizeFeature(canvas, cadDockWidget, mode),
      mCheckGeometryType(true) {
    if (canvas->currentLayer())
        qDebug() << tr("in QgsMapToolAddFeature: ")
                 << canvas->currentLayer()->name();
    setLayer(canvas->currentLayer());

    mToolName = tr("Add feature");
    // connect(QgisApp::instance(), &QgisApp::newProject, this,
    //         &QgsMapToolAddFeature::stopCapturing);
    // connect(QgisApp::instance(), &QgisApp::projectRead, this,
    //         &QgsMapToolAddFeature::stopCapturing);
}

bool QgsMapToolAddFeature::addFeature(QgsVectorLayer *vlayer,
                                      const QgsFeature &f, bool showModal) {
    qDebug() << tr("in addFeature: ") << vlayer->name();

    QgsFeature feat(f);
    QgsExpressionContextScope *scope =
        QgsExpressionContextUtils::mapToolCaptureScope(snappingMatches());
    QgsFeatureAction *action = new QgsFeatureAction(tr("add feature"), feat,
                                                    vlayer, QUuid(), -1, this);
    if (QgsRubberBand *rb = takeRubberBand())
        connect(action, &QgsFeatureAction::addFeatureFinished, rb,
                &QgsRubberBand::deleteLater);
    const bool res = action->addFeature(QgsAttributeMap(), showModal, scope);
    if (showModal) delete action;
    return res;
}

void QgsMapToolAddFeature::featureDigitized(const QgsFeature &feature) {
    QgsVectorLayer *vlayer = currentVectorLayer();
    qDebug() << tr("in featureDigitized: ") << vlayer->name();

    if (!addFeature(vlayer, feature, false)) {
        qDebug() << tr("QgsMapToolAddFeature.addFeature() fail! ")
                 << vlayer->name();
        return;
    }

    // add points to other features to keep topology up-to-date
    const bool topologicalEditing =
        QgsProject::instance()->topologicalEditing();
    const Qgis::AvoidIntersectionsMode avoidIntersectionsMode =
        QgsProject::instance()->avoidIntersectionsMode();
    if (topologicalEditing &&
        avoidIntersectionsMode ==
            Qgis::AvoidIntersectionsMode::AvoidIntersectionsLayers &&
        (mode() == CaptureLine || mode() == CapturePolygon)) {
        // use always topological editing for avoidIntersection.
        // Otherwise, no way to guarantee the geometries don't have a small
        // gap in between.
        const QList<QgsVectorLayer *> intersectionLayers =
            QgsProject::instance()->avoidIntersectionsLayers();

        if (!intersectionLayers.isEmpty())  // try to add topological points
                                            // also to background layers
        {
            for (QgsVectorLayer *vl : intersectionLayers) {
                // can only add topological points if background layer is
                // editable...
                if (vl->geometryType() == QgsWkbTypes::PolygonGeometry &&
                    vl->isEditable()) {
                    vl->addTopologicalPoints(feature.geometry());
                }
            }
        }
    }
    if (topologicalEditing) {
        const QList<QgsPointLocator::Match> sm = snappingMatches();
        for (int i = 0; i < sm.size(); ++i) {
            if (sm.at(i).layer() && sm.at(i).layer()->isEditable() &&
                sm.at(i).layer() != vlayer) {
                QgsPoint topologicalPoint{feature.geometry().vertexAt(i)};
                if (sm.at(i).layer()->crs() != vlayer->crs()) {
                    // transform digitized geometry from vlayer crs to
                    // snapping layer crs and add topological point
                    try {
                        topologicalPoint.transform(QgsCoordinateTransform(
                            vlayer->crs(), sm.at(i).layer()->crs(),
                            sm.at(i).layer()->transformContext()));
                        sm.at(i).layer()->addTopologicalPoints(
                            topologicalPoint);
                    } catch (QgsCsException &cse) {
                        Q_UNUSED(cse)
                        QgsDebugMsg(tr(
                            "transformation to layer coordinate failed"));
                    }
                } else {
                    sm.at(i).layer()->addTopologicalPoints(topologicalPoint);
                }
            }
        }
        vlayer->addTopologicalPoints(feature.geometry());
    }
}
