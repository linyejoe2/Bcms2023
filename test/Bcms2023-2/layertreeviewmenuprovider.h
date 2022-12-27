#ifndef LAYERTREEVIEWMENUPROVIDER_H
#define LAYERTREEVIEWMENUPROVIDER_H

#include <QObject>

#include <qgslayertreeview.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>

struct LegendLayerAction
{
	LegendLayerAction(QAction *a, QString m, QString i, bool all)
			: action(a), menu(m), id(i), allLayers(all) {}
	QAction *action;
	QString menu;
	QString id;
	bool allLayers;
	QList<QgsMapLayer *> layers;
};

class LayerTreeViewMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
	Q_OBJECT
public:
	LayerTreeViewMenuProvider(QgsLayerTreeView *view, QgsMapCanvas *canvas);

	// 覆寫右鍵選單
	QMenu *createContextMenu() override;

	void addLegendLayerAction(QAction *action, QString menu, QString id,
														int type, bool allLayers);
	bool removeLegendLayerAction(QAction *action);
	void addLegendLayerActionForLayer(QAction *action, QgsMapLayer *layer);
	void removeLegendLayerActionsForLayer(QgsMapLayer *layer);
	QList<LegendLayerAction> legendLayerActions(int type) const;

protected:
	void addCustomLayerActions(QMenu *menu, QgsMapLayer *layer);

	QgsLayerTreeView *layerList;
	QgsMapCanvas *mapCanvas;

	QMap<int, QList<LegendLayerAction>> mLegendLayerActionMap;
};

#endif // LAYERTREEVIEWMENUPROVIDER_H
