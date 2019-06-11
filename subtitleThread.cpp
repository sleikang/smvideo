#include "subtitleThread.h"
#include <QFile>
#include <qtextstream.h>
#include <QDebug>
#include <qregularexpression.h>
#include <QTime>
#include <QDir>

subtitleThread::subtitleThread(QObject *parent)
	: QThread(parent)
{
	readSubtitleMode = false;
	openMode = false;
}


subtitleThread::~subtitleThread()
{
}

//�����ļ���
void subtitleThread::setFileName(const QString & name)
{
	fileName = name;
}

//������Ļ
void subtitleThread::run()
{
	QFile file(fileName);
	if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
		emit loadStatus(QStringLiteral("�޷�������Ļ�ļ�"));
		file.close();
		return;
	}
	QTextStream in(&file);
	in.setCodec("UTF-8");
	txtList.clear();
	txtListEnd.clear();
	if (fileName.indexOf(QRegularExpression(".srt$")) != -1) {
		while (!in.atEnd()) {
			if (in.readLine().indexOf(QRegularExpression("^\\d+$")) != -1) {
				QString line = in.readLine();
				QRegularExpression rx("(\\d\\d:\\d\\d:\\d\\d)");
				QRegularExpressionMatch match = rx.match(line);
				QStringList timeList;
				while (match.hasMatch()) {
					timeList << match.captured(1);
					match = rx.match(line, match.capturedEnd());
				}
				line = in.readLine();
				QString text;
				while (!line.isEmpty()) {
					line.replace(QRegularExpression("{.*?}"), "");
					if (!text.isEmpty())
						text += "\n";
					text += line;
					line = in.readLine();
				}
				txtList[timeList.at(0)] = text;
				txtListEnd[timeList.at(1)] = "";
			}
		}
	}
	else if (fileName.indexOf(QRegularExpression(".ass$")) != -1) {
		while (!in.atEnd()) {
			QString line = in.readLine();
			if (line.indexOf("Dialogue:") != -1) {
				QRegularExpression rx("(\\d:\\d\\d:\\d\\d)");
				QRegularExpressionMatch match = rx.match(line);
				QStringList timeList;
				while (match.hasMatch()) {
					timeList << match.captured(1).insert(0, "0");
					match = rx.match(line, match.capturedEnd());
				}
				line.replace(QRegularExpression("Dialogue:.*,,"), "").replace(QRegularExpression("^{.*?}$|{.*?}"), "").replace("\\N", "\n");
				txtList[timeList.at(0)] = line;
				txtListEnd[timeList.at(1)] = "";
			}
		}
	}
	file.close();
	openMode = true;
	timeControl = 0;
	emit loadStatus(QStringLiteral("������Ļ�ļ��ɹ�"));
	exec();  //�����߳�ѭ��
}

//���ҵ�ǰĿ¼�Ƿ�����Ļ�ļ�
void subtitleThread::findFile(const QString &name)
{
	QString path = name;
	QString tempFileName = path.split("/").at(path.split("/").size() - 1);
	tempFileName.replace(QRegularExpression("(\\.)\\S{1,4}$"), "");
	path.replace(path.split("/").at(path.split("/").size() - 1), "");
	QDir dir(path);
	QStringList list;
	list << "*.ass" << "*.srt";
	dir.setNameFilters(list);
	list = dir.entryList();
	for (int i = 0; i < list.size(); i++) {
		if (list.at(i).indexOf(tempFileName) != -1) {
			fileName = path + list.at(i);
			emit haveFound(true);
			emit loadStatus(QStringLiteral("�ҵ�������Ļ�ļ�"));
			return;
		}
	}
	emit haveFound(false);
}

//�����Ļ
void subtitleThread::clearSubtitle()
{
	txtList.clear();
	txtListEnd.clear();
}

//��ǰʱ��
int subtitleThread::addTime()
{
	return (timeControl += 100);
}

//�ӳ�ʱ��
int subtitleThread::lowerTime()
{
	return (timeControl -= 100);
}

void subtitleThread::readTime(qint64 position)  //���Ž���ƥ����Ļ
{
	if (openMode) {
		if (!readSubtitleMode) {
			QString time = QTime::fromMSecsSinceStartOfDay(position + timeControl).toString("hh:mm:ss");
			if (!txtList.value(time).isNull()) {
				emit readSubtitleText(txtList.value(time));
				readSubtitleMode = true;
			}
		}
		else {
			QString time = QTime::fromMSecsSinceStartOfDay(position + timeControl).toString("hh:mm:ss");
			if (!txtListEnd.value(time).isNull()) {
				emit readSubtitleText("");
				readSubtitleMode = false;
			}

			if (!txtList.value(time).isNull()) {
				emit readSubtitleText(txtList.value(time));
				readSubtitleMode = true;
			}
		}
	}
}