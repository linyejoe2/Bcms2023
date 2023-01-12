#include <QApplication>

#include "bcmsapp.h"
#include <qtextedit.h>

int main(int argc, char *argv[]) {
    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    BcmsApp w;

    // qDebug() << "keys--" << QStyleFactory::keys();
    w.show();
    return a.exec();
}
