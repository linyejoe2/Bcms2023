#include "bcmsmainwindow.h"
#include "ui_bcmsmainwindow.h"

BcmsMainWindow::BcmsMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BcmsMainWindow)
{
    ui->setupUi(this);
}

BcmsMainWindow::~BcmsMainWindow()
{
    delete ui;
}

