#ifndef GEOVIEWER_H
#define GEOVIEWER_H

#include <QJsonArray>
#include <QObject>
#include <QString>

class GeoViewer : public QObject {
    Q_OBJECT

   public:
    static GeoViewer* instance() {
        if (!ins) {
            ins = new GeoViewer();
        }
        return ins;
    }

    static QJsonArray* getFeaturedef() { return featuredef.get(); }

    /**
     * @brief 連接 api
     *
     * @param path 要連接的 api ex: "/api/v3/featuredef/get"
     * @return QString 返回文字
     */
    QString connectApi(QString path);

   private:
    GeoViewer();

    static GeoViewer* ins;

    //! 圖層設定
    static std::unique_ptr<QJsonArray> featuredef;
};

// GeoViewer* GeoViewer::instance = nullptr;

#endif  // GEOVIEWER_H
