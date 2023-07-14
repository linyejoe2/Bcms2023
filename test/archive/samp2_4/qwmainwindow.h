#ifndef QWMAINWINDOW_H
#define QWMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class QWMainWindow; }
QT_END_NAMESPACE

class QWMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QWMainWindow(QWidget *parent = nullptr);
    ~QWMainWindow();

private:
    Ui::QWMainWindow *ui;
};
#endif // QWMAINWINDOW_H
