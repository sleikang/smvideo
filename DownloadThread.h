#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class DownloadThread : public QObject
{
	Q_OBJECT

public:
	DownloadThread(QObject *parent = nullptr);
	~DownloadThread();
	void setFileName(const QString &str);  //设置字幕文件名
	void run();  //开始搜索
	void setDownMode(int mode);  //设置下载模式
	void loadRowDown(int row);  //下载选中字幕

private:
	QString fileName;  //保存字幕文件名
	QNetworkAccessManager *manager;  //HTTP/HTTPS访问
	QNetworkRequest *request; //协议请求头
	int readNum;  //加载次数
	QList<QStringList *> fileList;
	QString fileType; //保存文件类型
	int downMode;  //下载模式

private slots:
	void readUrlValue(QNetworkReply *reply);  //返回数据

signals:
	void readFileName(const QString &str);  //字幕文件名信号
	void loadStatus(const QString &str); //加载状态
	void readListWidget(QList<QStringList *> file);  //加载列表窗口信号
};
