#include "layertreeviewmenuprovider.h"
#include "mainwindow.h"

#include <QMenu>
#include <QModelIndex>
#include <QIcon>

// QGis include
#include <qgslayertreeviewdefaultactions.h>
#include <qgslayertreenode.h>
#include <qgslayertreemodel.h>
#include <qgslayertree.h>
#include <qgsrasterlayer.h>

LayerTreeViewMenuProvider::LayerTreeViewMenuProvider(QgsLayerTreeView *view, QgsMapCanvas *canvas)
		: layerList(view), mapCanvas(canvas) {}

QMenu *LayerTreeViewMenuProvider::createContextMenu()
{
	QMenu *menu = new QMenu;

	QgsLayerTreeViewDefaultActions *actions = layerList->defaultActions();

	QModelIndex modelIndex = layerList->currentIndex();

	// global menu
	if (!modelIndex.isValid())
	{
		menu->addAction(actions->actionAddGroup(menu));
	}

	return menu;
}

void LayerTreeViewMenuProvider::addLegendLayerAction( QAction * action, QString menu, QString id, int type, bool allLayers )
{
 
}
 
void LayerTreeViewMenuProvider::addLegendLayerActionForLayer( QAction * action, QgsMapLayer * layer )
{
 
}
 
void LayerTreeViewMenuProvider::removeLegendLayerActionsForLayer( QgsMapLayer * layer )
{
 
}

QList< LegendLayerAction > LayerTreeViewMenuProvider::legendLayerActions( int type ) const
{
    return mLegendLayerActionMap.contains( type ) ? mLegendLayerActionMap.value( type ) : QList<LegendLayerAction>();
}
