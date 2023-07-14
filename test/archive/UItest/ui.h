#ifndef UI_H
#define UI_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class UI; }
QT_END_NAMESPACE

class UI : public QMainWindow
{
    Q_OBJECT

public:
    UI(QWidget *parent = nullptr);
    ~UI();

private slots:
    void on_printS_clicked(bool checked);

private:
    Ui::UI *ui;
};
#endif // UI_H
