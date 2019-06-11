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
	~DownMovieInfo(); //��ʼ��ѯ��Ӱ��Ϣ
	void run();
	void setFileName(const QString &str);  //�����ļ���

private:
	QString fileName;  //��Ӱ����
	QNetworkAccessManager *manager;  //HTTP/HTTPS����
	QNetworkRequest *request; //Э������ͷ
	int readNum;  //���ش���
	QLabel *labelImage;  //ͼƬ
	QLabel *labelInfo;  //��Ϣ
	QString info;  //������ʱ��Ϣ
	QString plot;  //������ʱ����

private slots:
	void movieInfo(QNetworkReply *reply);  //���ز�ѯ��Ϣ

signals:
	void loadStatus(const QString &str); //����״̬
	void readTextPlot(const QString &str);  //���;�����Ϣ
	void loadFinish();  //�������
};
