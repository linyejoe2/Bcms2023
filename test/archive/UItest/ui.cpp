#include "ui.h"
#include "ui_ui.h"
#include <iostream>

UI::UI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UI)
{
    ui->setupUi(this);
    QGraphicsView testView = ui->gView2;
}

UI::~UI()
{
    delete ui;
}

//void UI::on_printS_clicked()
//{
//    std::cout << "測試測試TEST";
//}


void UI::on_printS_clicked(bool checked)
{
    std::cout << "測試測試TEST";
}

