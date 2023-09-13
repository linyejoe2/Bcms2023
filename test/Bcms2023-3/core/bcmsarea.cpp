#include "bcmsarea.h"

BcmsArea::BcmsArea(QObject *parent) : QObject{parent} {
    mAreaListItemModel->setHorizontalHeaderLabels(
        {tr("狀態"), tr("法定空地編號"), tr("字串")});
}

void BcmsArea::addArea(IAreaObject2 *areaObj) {
    mAreaList.append(areaObj);
    mEditingArea = areaObj;
    updateAreaListItemModel();
}

void BcmsArea::doneEditing() {
    mEditingArea->editing = false;
    updateAreaListItemModel();
}

void BcmsArea::updateAreaListItemModel() {
    mAreaListItemModel->clear();
    foreach (auto &ele, mAreaList) {
        QList<QStandardItem *> r;
        r.append(new QStandardItem(ele->editing ? tr("編輯中") : tr("待上傳")));
        r.append(new QStandardItem(ele->areaKey));
        r.append(new QStandardItem(ele->area.geometry().asJson()));
        if (ele->editing) {
            foreach (auto i, r) {
                i->setBackground(QBrush(QColor("#e0edfb")));
            };
        }
        mAreaListItemModel->appendRow(r);
    }
}
