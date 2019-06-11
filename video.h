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
	void readSettings();  //读取配置文件
	void writeSettings();  //写入配置文件

private:
	Ui::videoClass ui;
	QMediaPlayer *player;  //视频播放
	VideoWidget *videoWidget;  //视频播放窗口
	bool playerStatus;  //播放状态
	QTimer *timer;  //全屏状态栏显示定时器
	bool showFullMenuMode;   //全屏菜单显示状态
	QGridLayout *gridLayout;  //播放窗口布局
	QString fileSubtitle; //字幕文件名
	subtitleThread *subtitle; //字幕加载线程
	QLabel *label;  //显示字幕
	DownloadThread *download;  //字幕下载
	QThread *downThread;  //字幕下载线程
	QString fileName;  //保存打开文件
	ListWidget *listWidget;  //字幕选择下载窗口
	bool downFinish;  //是否已下载
	int volume; //保存音量
	QStringList fileNameList;  //保存最近文件列表
	//QStringList audioList;  //保存音频流列表
	bool positionMove;  //进度条是否在移动
	bool fullShow;  //是否全屏 是否在显示字幕下窗口
	bool downSubtitleMode; //是开始下载字幕
	QStringList folderList;  //保存文件夹列表
	QStringList folderFileList;  //保存目录文件列表
	QMap<QString, QString> folderFilePath;  //保存文件路径用来快速读取电影目录
	QMenu *right_2;  //右键菜单
	DownMovieInfo *downMovieInfo;  //下载海报
	QThread *downMovieInfoThread;  //下载海报线程
	QAction *addAction_2;  //添加目录
	QAction *deleteAction_2;  //删除目录

private slots:
	void on_pushButton_clicked();  //播放按钮
	void on_pushButton_2_clicked();  //静音
	void setSliderValue(qint64 position);  //设置进度
	void openPositionMove();   //开启进度条移动状态
	void closePositionMove();   //关闭进度条移动状态
	void openFile(); //打开文件
	void showFullMenu();  //显示播放菜单
	void hideFullMenu();  //隐藏全屏菜单
	void hideFull();  //隐藏全屏
	void showFull();  //显示全屏
	void openSubtitleFile();  //打开字幕文件
	void showSubtitle(bool checked);  //显示字幕
	void readSubtitleText(const QString &str);  //读取字幕
	void setVolumeIcon(int position);  //设置音量图标
	void keyPress(int key);  //按键操作
	void loadMedia(QMediaPlayer::MediaStatus status);  //加载视频完成
	void loadSubtitleFile(const QString &str);  //网络下载完成加载字幕文件
	void loadLocalSubtitleFile(bool status);  //加载本地字幕文件
	void autoLoadSubtitle(bool checked);  //自动在线匹配字幕开始
	void searchLoadSubtitle(bool checked);  //搜索匹配在线字幕
	void startSearch();  //开始搜索
	void showMessageLabel(const QString &str);  //显示消息
	void showQListWidget(QList<QStringList *> file);  //显示列表窗口
	void closeDownWighet();  //关闭下载字幕窗口
	void addSubtitleTime();  //字幕时间提前
	void lowerSubtitleTime();  //字幕时间延迟
	void downFinished();  //字幕下载完成后 字幕线程成功退出
	void openRecentFile(QAction  *action);  //打开最近文件
	void showList(bool checked);  //是否显示目录
	void showRightMenu_2(const QPoint & pos);  //显示右键菜单
	void listWidgetOption_2(QAction * action);  //菜单关联
	void showPathFile(int row);  //显示目录文件
	void showMovieInfo(int row);  //显示电影信息
	void playerMovie(const QModelIndex &index);  //双击目录电影项播放视频

protected:
	void moveEvent(QMoveEvent *event);  //窗口移动事件
	void resizeEvent(QResizeEvent *event);  //窗口大小改变事件
	void closeEvent(QCloseEvent *event);  //窗口关闭事件
	bool eventFilter(QObject *watched, QEvent *event);//事件过滤器 
};
