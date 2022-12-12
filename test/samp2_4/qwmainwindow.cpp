#include "qwmainwindow.h"
#include "ui_qwmainwindow.h"

QWMainWindow::QWMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QWMainWindow)
{
    ui->setupUi(this);
}

QWMainWindow::~QWMainWindow()
{
    delete ui;
}

