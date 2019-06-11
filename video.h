#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_video.h"
#include <qmediaplayer.h>

class VideoWidget;
class QTimer;
class QGridLayout;
class subtitleThread;
class QLabel;
class DownloadThread;
class QThread;
class ListWidget;
class QMenu;
class DownMovieInfo;

class video : public QMainWindow
{
	Q_OBJECT

public:
	video(QWidget *parent = Q_NULLPTR);
	~video();
	void readSettings();  //��ȡ�����ļ�
	void writeSettings();  //д�������ļ�

private:
	Ui::videoClass ui;
	QMediaPlayer *player;  //��Ƶ����
	VideoWidget *videoWidget;  //��Ƶ���Ŵ���
	bool playerStatus;  //����״̬
	QTimer *timer;  //ȫ��״̬����ʾ��ʱ��
	bool showFullMenuMode;   //ȫ���˵���ʾ״̬
	QGridLayout *gridLayout;  //���Ŵ��ڲ���
	QString fileSubtitle; //��Ļ�ļ���
	subtitleThread *subtitle; //��Ļ�����߳�
	QLabel *label;  //��ʾ��Ļ
	DownloadThread *download;  //��Ļ����
	QThread *downThread;  //��Ļ�����߳�
	QString fileName;  //������ļ�
	ListWidget *listWidget;  //��Ļѡ�����ش���
	bool downFinish;  //�Ƿ�������
	int volume; //��������
	QStringList fileNameList;  //��������ļ��б�
	//QStringList audioList;  //������Ƶ���б�
	bool positionMove;  //�������Ƿ����ƶ�
	bool fullShow;  //�Ƿ�ȫ�� �Ƿ�����ʾ��Ļ�´���
	bool downSubtitleMode; //�ǿ�ʼ������Ļ
	QStringList folderList;  //�����ļ����б�
	QStringList folderFileList;  //����Ŀ¼�ļ��б�
	QMap<QString, QString> folderFilePath;  //�����ļ�·���������ٶ�ȡ��ӰĿ¼
	QMenu *right_2;  //�Ҽ��˵�
	DownMovieInfo *downMovieInfo;  //���غ���
	QThread *downMovieInfoThread;  //���غ����߳�
	QAction *addAction_2;  //���Ŀ¼
	QAction *deleteAction_2;  //ɾ��Ŀ¼

private slots:
	void on_pushButton_clicked();  //���Ű�ť
	void on_pushButton_2_clicked();  //����
	void setSliderValue(qint64 position);  //���ý���
	void openPositionMove();   //�����������ƶ�״̬
	void closePositionMove();   //�رս������ƶ�״̬
	void openFile(); //���ļ�
	void showFullMenu();  //��ʾ���Ų˵�
	void hideFullMenu();  //����ȫ���˵�
	void hideFull();  //����ȫ��
	void showFull();  //��ʾȫ��
	void openSubtitleFile();  //����Ļ�ļ�
	void showSubtitle(bool checked);  //��ʾ��Ļ
	void readSubtitleText(const QString &str);  //��ȡ��Ļ
	void setVolumeIcon(int position);  //��������ͼ��
	void keyPress(int key);  //��������
	void loadMedia(QMediaPlayer::MediaStatus status);  //������Ƶ���
	void loadSubtitleFile(const QString &str);  //����������ɼ�����Ļ�ļ�
	void loadLocalSubtitleFile(bool status);  //���ر�����Ļ�ļ�
	void autoLoadSubtitle(bool checked);  //�Զ�����ƥ����Ļ��ʼ
	void searchLoadSubtitle(bool checked);  //����ƥ��������Ļ
	void startSearch();  //��ʼ����
	void showMessageLabel(const QString &str);  //��ʾ��Ϣ
	void showQListWidget(QList<QStringList *> file);  //��ʾ�б���
	void closeDownWighet();  //�ر�������Ļ����
	void addSubtitleTime();  //��Ļʱ����ǰ
	void lowerSubtitleTime();  //��Ļʱ���ӳ�
	void downFinished();  //��Ļ������ɺ� ��Ļ�̳߳ɹ��˳�
	void openRecentFile(QAction  *action);  //������ļ�
	void showList(bool checked);  //�Ƿ���ʾĿ¼
	void showRightMenu_2(const QPoint & pos);  //��ʾ�Ҽ��˵�
	void listWidgetOption_2(QAction * action);  //�˵�����
	void showPathFile(int row);  //��ʾĿ¼�ļ�
	void showMovieInfo(int row);  //��ʾ��Ӱ��Ϣ
	void playerMovie(const QModelIndex &index);  //˫��Ŀ¼��Ӱ�����Ƶ

protected:
	void moveEvent(QMoveEvent *event);  //�����ƶ��¼�
	void resizeEvent(QResizeEvent *event);  //���ڴ�С�ı��¼�
	void closeEvent(QCloseEvent *event);  //���ڹر��¼�
	bool eventFilter(QObject *watched, QEvent *event);//�¼������� 
};
