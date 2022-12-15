#ifndef BCMSMAINWINDOW_H
#define BCMSMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class BcmsMainWindow; }
QT_END_NAMESPACE

class BcmsMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    BcmsMainWindow(QWidget *parent = nullptr);
    ~BcmsMainWindow();

private:
    Ui::BcmsMainWindow *ui;
};
#endif // BCMSMAINWINDOW_H
