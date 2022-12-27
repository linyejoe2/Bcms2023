#include "maptools/qgsappmaptools.h"
#include <qgsmaptool.h>
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolpan.h>
// #include <qgsmaptool
#include <qgssettingsregistrycore.h>
#include <qgsmaptoolmodifyannotation.h>

#include "qgsmaptoolselect.h"
#include "mainwindow.h"

QgsAppMapTools::QgsAppMapTools(QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock)
{
  mTools.insert(Tool::ZoomIn, new QgsMapToolZoom(canvas, false /* zoomIn */));
  mTools.insert(Tool::ZoomOut, new QgsMapToolZoom(canvas, true /* zoomOut */));
  mTools.insert(Tool::Pan, new QgsMapToolPan(canvas));
  mTools.insert(Tool::AnnotationEdit, new QgsMapToolModifyAnnotation(canvas, cadDock));
  mTools.insert( Tool::SelectFeatures, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectPolygon, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectFreehand, new QgsMapToolSelect( canvas ) );
  mTools.insert( Tool::SelectRadius, new QgsMapToolSelect( canvas ) );
}

QgsAppMapTools::~QgsAppMapTools()
{
  for (auto it = mTools.constBegin(); it != mTools.constEnd(); ++it)
  {
    delete it.value();
  }
}

QgsMapTool *QgsAppMapTools::mapTool(QgsAppMapTools::Tool tool)
{
  return mTools.value(tool);
}

// QList<QgsMapToolCapture *> QgsAppMapTools::captureTools() const
// {
//   QList<QgsMapToolCapture *> res;
//   for (auto it = mTools.constBegin(); it != mTools.constEnd(); ++it)
//   {
//     if (QgsMapToolCapture *captureTool = qobject_cast<QgsMapToolCapture *>(it.value()))
//       res << captureTool;
//   }
//   return res;
// }
