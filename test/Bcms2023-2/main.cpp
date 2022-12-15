#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	QMainWindow main_window;

	w.show();
	return a.exec();
}
