#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QLabel;

class DownMovieInfo : public QObject
{
	Q_OBJECT

public:
	DownMovieInfo(QObject *parent = nullptr);
	DownMovieInfo(QLabel *la, QLabel *lb, QObject *parent = nullptr);
	~DownMovieInfo(); //开始查询电影信息
	void run();
	void setFileName(const QString &str);  //设置文件名

private:
	QString fileName;  //电影名称
	QNetworkAccessManager *manager;  //HTTP/HTTPS访问
	QNetworkRequest *request; //协议请求头
	int readNum;  //加载次数
	QLabel *labelImage;  //图片
	QLabel *labelInfo;  //信息
	QString info;  //保存临时信息
	QString plot;  //保存临时剧情

private slots:
	void movieInfo(QNetworkReply *reply);  //返回查询信息

signals:
	void loadStatus(const QString &str); //加载状态
	void readTextPlot(const QString &str);  //发送剧情信息
	void loadFinish();  //加载完成
};
