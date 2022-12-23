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
		menu->addAction(QIcon("mActionExpandTree.png"), tr("&Expand All"), layerList, SLOT(expandAll()));
		menu->addAction(QIcon("mActionCollapseTree.png"), tr("&Collapse All"), layerList, SLOT(collapseAll()));
	}
	else
	{
		// QgsLayerTreeNode *node1 = layerList->layerTreeModel()->index2node(modelIndex);
		QgsLayerTreeNode *node1 = layerList->index2node(modelIndex);

		std::cout << node1 << endl;

		// layer or group selected
		if (node1 && QgsLayerTree::isGroup(node1))
		{
			menu->addAction(actions->actionZoomToGroup(mapCanvas, menu));
			menu->addAction(actions->actionRenameGroupOrLayer(menu));
			if (layerList->selectedNodes(true).count() >= 2)
			{
				menu->addAction(actions->actionGroupSelected(menu));
			}
			menu->addAction(actions->actionAddGroup(menu));
		}
		else if (QgsLayerTree::isLayer(node1))
		{
			QgsMapLayer *layer = QgsLayerTree::toLayer(node1)->layer();
			menu->addAction(actions->actionZoomToLayers(mapCanvas, menu));
			menu->addAction(actions->actionShowInOverview(menu));
		}
	}
	return menu;
}

void LayerTreeViewMenuProvider::addLegendLayerAction(QAction *action, QString menu, QString id, int type, bool allLayers)
{
}

void LayerTreeViewMenuProvider::addLegendLayerActionForLayer(QAction *action, QgsMapLayer *layer)
{
}

void LayerTreeViewMenuProvider::removeLegendLayerActionsForLayer(QgsMapLayer *layer)
{
}

QList<LegendLayerAction> LayerTreeViewMenuProvider::legendLayerActions(int type) const
{
	return mLegendLayerActionMap.contains(type) ? mLegendLayerActionMap.value(type) : QList<LegendLayerAction>();
}
