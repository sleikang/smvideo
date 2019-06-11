#pragma once

#include <QThread>
#include <QMap>

class subtitleThread : public QThread
{
	Q_OBJECT

public:
	subtitleThread(QObject *parent = nullptr);
	~subtitleThread();
	void setFileName(const QString &name);  //设置文件名
	void run();
	void findFile(const QString &name);  //查找当前目录是否有字幕文件
	void clearSubtitle();  //清除字幕
	int addTime();  //提前时间
	int lowerTime();  //延迟时间

private:
	QString fileName;  //保存字幕文件
	QString pathFile;  //保存打开视频文件路径
	QMap<QString, QString> txtList;  //保存解析字幕内容
	QMap<QString, QString> txtListEnd; //保存解析字幕结束时间
	bool readSubtitleMode;  //当前是否加载了字幕
	bool openMode;  //当前字幕开关
	int timeControl;  //字幕时间

public slots:
	void readTime(qint64 position);  //当前播放进度

signals:
	void readSubtitleText(const QString &str);  //发送字幕信号
	void haveFound(bool status);  //找到字幕文件
	void loadStatus(const QString &str); //加载状态
};
