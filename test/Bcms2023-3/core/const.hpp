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
    QSettings *setting;
    

    CONST() {
        setting = new QSettings("./Bcms2023.ini", QSettings::IniFormat);
    };
};

#endif  // CONST_H
