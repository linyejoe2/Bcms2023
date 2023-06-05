#ifndef CONST_H
#define CONST_H

#include <QObject>
#include <QSettings>
#include <QString>

class CONST {
   public:
    static CONST *instance() {
        static CONST inst;
        return &inst;
    }

    QString getGeoViewerJSServiceUrl() {
        return setting->value("geoViewerJSServiceUrl").toString();
    }

   private:
    CONST() {
        setting = new QSettings("./Bcms2023.ini", QSettings::IniFormat);
        // setting->setValue("geoViewerJSServiceUrl", "asdfas");
        // qDebug() << QObject::tr("url: ") <<
        // setting->value("geoViewerJSServiceUrl").toString();
    };
    QSettings *setting;
};

#endif  // CONST_H
