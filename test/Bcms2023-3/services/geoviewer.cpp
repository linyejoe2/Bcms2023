#include "geoviewer.h"

#include <QEventLoop>
// #include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItemModel>

#include "../core/const.hpp"

GeoViewer* GeoViewer::ins = nullptr;
QJsonArray* GeoViewer::featuredef = nullptr;

GeoViewer::GeoViewer() {
    //! init featuredef
    *featuredef =
        QJsonDocument::fromJson(connectApi("/api/v3/featuredef/get").toUtf8())
            .array();
}

QString GeoViewer::connectApi(QString path) {
    //! defind variable
    QString url = CONST::instance()->getGeoViewerJSServiceUrl();

    //! fetch ZON
    QNetworkAccessManager manager;
    QNetworkReply* res = manager.get(QNetworkRequest(url + path));
    QEventLoop event;
    connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    return res->readAll();
}