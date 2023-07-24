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
std::unique_ptr<QJsonArray> GeoViewer::featuredef = nullptr;

GeoViewer::GeoViewer() {
    try {
        //! init featuredef
        auto temp = connectApi("/api/v3/featuredef/get").toUtf8();
        // qDebug() << "featuredef raw data: " << temp;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(temp);

        if (jsonResponse.isArray()) {
            // QJsonArray array = jsonResponse.array();
            GeoViewer::featuredef =
                std::make_unique<QJsonArray>(jsonResponse.array());
            // QString fdesc = GeoViewer::featuredef->at(0)
            //                     .toObject()
            //                     .value("fdesc")
            //                     .toString();
            // qDebug() << "fdesc: " << fdesc;

            qDebug() << "Featuredef initialized successfully.";
        } else {
            qDebug() << "Error: Invalid JSON response for featuredef.";
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception occurred while initializing featuredef:"
                 << e.what();
    }
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