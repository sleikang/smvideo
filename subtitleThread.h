#pragma once

#include <QThread>
#include <QMap>

class subtitleThread : public QThread
{
	Q_OBJECT

public:
	subtitleThread(QObject *parent = nullptr);
	~subtitleThread();
	void setFileName(const QString &name);  //�����ļ���
	void run();
	void findFile(const QString &name);  //���ҵ�ǰĿ¼�Ƿ�����Ļ�ļ�
	void clearSubtitle();  //�����Ļ
	int addTime();  //��ǰʱ��
	int lowerTime();  //�ӳ�ʱ��

private:
	QString fileName;  //������Ļ�ļ�
	QString pathFile;  //�������Ƶ�ļ�·��
	QMap<QString, QString> txtList;  //���������Ļ����
	QMap<QString, QString> txtListEnd; //���������Ļ����ʱ��
	bool readSubtitleMode;  //��ǰ�Ƿ��������Ļ
	bool openMode;  //��ǰ��Ļ����
	int timeControl;  //��Ļʱ��

public slots:
	void readTime(qint64 position);  //��ǰ���Ž���

signals:
	void readSubtitleText(const QString &str);  //������Ļ�ź�
	void haveFound(bool status);  //�ҵ���Ļ�ļ�
	void loadStatus(const QString &str); //����״̬
};
