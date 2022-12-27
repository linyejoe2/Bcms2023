#include "mainwindow.h"

#include <QApplication>

#include <qgsapplication.h>

int main(int argc, char *argv[])
{
	// QgsApplication a( argc, argv, true );
	QApplication a(argc, argv);
	MainWindow w;

	QMainWindow main_window;

	w.show();
	return a.exec();
}
