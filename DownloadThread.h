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
	void setFileName(const QString &str);  //������Ļ�ļ���
	void run();  //��ʼ����
	void setDownMode(int mode);  //��������ģʽ
	void loadRowDown(int row);  //����ѡ����Ļ

private:
	QString fileName;  //������Ļ�ļ���
	QNetworkAccessManager *manager;  //HTTP/HTTPS����
	QNetworkRequest *request; //Э������ͷ
	int readNum;  //���ش���
	QList<QStringList *> fileList;
	QString fileType; //�����ļ�����
	int downMode;  //����ģʽ

private slots:
	void readUrlValue(QNetworkReply *reply);  //��������

signals:
	void readFileName(const QString &str);  //��Ļ�ļ����ź�
	void loadStatus(const QString &str); //����״̬
	void readListWidget(QList<QStringList *> file);  //�����б����ź�
};
