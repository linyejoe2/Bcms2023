#ifndef BCMSAREA_H
#define BCMSAREA_H

// Qt
#include <QList>
#include <QObject>
#include <QStandardItemModel>
#include <QStringLiteral>
#include <QUuid>

// Qgis
#include <qgsfeature.h>

struct IAreaObject2 {
    bool editing;
    QString areaKey = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QgsFeature area;
    QString filePath;
};

/**
 * @brief
 * 法空空間(建築物)
 *
 * 所有跟 area
 * 有關的東西都儲存在這邊，新增或編輯一個建築物之後就會加入一筆資料到 mAreaList
 * 裡，上傳到伺服器之後從 mAreaList 裡刪除。
 * @note singlaton
 */
class BcmsArea : public QObject {
    Q_OBJECT
   public:
    BcmsArea(const BcmsArea&) = delete;
    BcmsArea& operator=(const BcmsArea&) = delete;
    explicit BcmsArea(QObject* parent = nullptr);
    static BcmsArea& instance() {
        static BcmsArea i;
        return i;
    }

    QList<IAreaObject2*> areaList() { return mAreaList; };
    IAreaObject2* editingArea() { return mEditingArea; };
    QStandardItemModel* areaListItemModel() { return mAreaListItemModel; };
    /**
     * @brief 完成編輯: 清空 mEditingArea ，修改 areaObj.editing 為 false ，更新 mAreaListItemModel 。
     */
    void doneEditing();
    /**
     * @brief 新增法空: 新增一個 areaObj ，放入 mAreaList ，並加入 mEditingArea 中。
     * @param areaObj 
     */
    void addArea(IAreaObject2* areaObj);
    //@ 更新 model
    void updateAreaListItemModel();

   private:
    QList<IAreaObject2*> mAreaList;
    IAreaObject2* mEditingArea;
    QStandardItemModel* mAreaListItemModel = new QStandardItemModel(this);

   signals:
};

#endif  // BCMSAREA_H
