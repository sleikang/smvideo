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

//设置文件名
void subtitleThread::setFileName(const QString & name)
{
	fileName = name;
}

//加载字幕
void subtitleThread::run()
{
	QFile file(fileName);
	if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
		emit loadStatus(QStringLiteral("无法加载字幕文件"));
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
	emit loadStatus(QStringLiteral("加载字幕文件成功"));
	exec();  //保持线程循环
}

//查找当前目录是否有字幕文件
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
			emit loadStatus(QStringLiteral("找到本地字幕文件"));
			return;
		}
	}
	emit haveFound(false);
}

//清除字幕
void subtitleThread::clearSubtitle()
{
	txtList.clear();
	txtListEnd.clear();
}

//提前时间
int subtitleThread::addTime()
{
	return (timeControl += 100);
}

//延迟时间
int subtitleThread::lowerTime()
{
	return (timeControl -= 100);
}

void subtitleThread::readTime(qint64 position)  //播放进度匹配字幕
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