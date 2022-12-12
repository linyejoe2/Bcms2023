#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <QDir>
#include <qgsapplication.h>

int main(int argc, char *argv[])
{
    QgsApplication a(argc, argv, true);
    QString cpath = QDir::currentPath();

    QgsApplication::initQgis();

    MainWindow w;
    w.show();
    return a.exec();
}
