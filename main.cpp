#include "video.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//╪стьсОятнд╪Ч
	QTranslator translator;
	translator.load(":/language/qt_zh_CN.qm");
	a.installTranslator(&translator);

	video w;
	w.show();
	return a.exec();
}
