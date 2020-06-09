#include "geneticpathfinder.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	geneticPathFinder w;
	w.show();
	return a.exec();
}
