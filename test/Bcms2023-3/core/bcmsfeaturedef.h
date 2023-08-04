#ifndef BCMSFEATUREDEF_H
#define BCMSFEATUREDEF_H

#include <qgsrenderer.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

#include "qdebug.h"

class BcmsFeaturedef : public QObject {
    Q_OBJECT

   public:
    BcmsFeaturedef(const BcmsFeaturedef&) = delete;
    BcmsFeaturedef& operator=(const BcmsFeaturedef&) = delete;
    explicit BcmsFeaturedef(QObject* parent = nullptr);
    static BcmsFeaturedef& instance() {
        static BcmsFeaturedef ins;
        return ins;
    }

    QString rowData() { return __object.toJson(QJsonDocument::Indented); }

    QJsonObject getSymbolConfig(const QString& fname);

    QString getSymbolDesc(const QString& fname);

    QString getSymbolZIndex(const QString& fname);

    QgsFeatureRenderer* getSymbolRenderer(const QString& fname);

    QVector<QString>* getIdList() { return &__idList; };

   signals:

   private:
    //! 圖層設定
    // QByteArray __rowData;

    QJsonDocument __object;

    QVector<QString> __idList = {} ;
#ifdef DEVELOPING

   private:
    ~BcmsFeaturedef() { qDebug() << "BcmsFeaturedef destructor called!"; }
#endif
};

#endif  // BCMSFEATUREDEF_H
