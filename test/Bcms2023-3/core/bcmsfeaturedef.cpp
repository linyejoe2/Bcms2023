#include "bcmsfeaturedef.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "../services/geoviewer.h"

BcmsFeaturedef::BcmsFeaturedef(QObject *parent) : QObject{parent} {
    //! init featuredef
    auto temp =
        GeoViewer::instance()->connectApi("/api/v3/featuredef/get").toUtf8();
    qDebug() << "featuredef raw data: " << temp;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(temp);
    // QString temp = jsonResponse.toJson(QJsonDocument::Indented);

    __rowData = new QString(jsonResponse.toJson(QJsonDocument::Indented));
    qDebug() << "rowData: " << __rowData;

    
}