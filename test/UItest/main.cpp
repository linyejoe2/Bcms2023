#define _USE_MATH_DEFINES
#include "ui.h"
#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    std::cout << "測試測試";


    UI w;
    w.show();
    return a.exec();
}
