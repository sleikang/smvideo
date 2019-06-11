#include "video.h"
#include <qvideowidget.h>
#include <QTime>
#include <qfiledialog.h>
#include <QMouseEvent>
#include "VideoWidget.h"
#include <QTimer>
#include <qgridlayout.h>
#include "subtitleThread.h"
#include <QLabel>
#include <qmessagebox.h>
#include "DownloadThread.h"
#include <QThread>
#include <QListWidget>
#include "ListWidget.h"
#include <qsettings.h>
#include <Windows.h>
#include <QMenu>
#include "DownMovieInfo.h"

video::video(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	player = new QMediaPlayer(this);
	videoWidget = new VideoWidget;
	videoWidget->setMouseTracking(true);
	videoWidget->setFocusPolicy(Qt::ClickFocus);  //设置按键单击
	player->setVideoOutput(videoWidget);
	gridLayout = new QGridLayout(ui.widget);  //视频播放单窗口布局
	gridLayout->addWidget(videoWidget);
	gridLayout->setContentsMargins(0, 0, 0, 0); //设置布局边距
	ui.gridLayout->setContentsMargins(0, 0, 0, 0);
	videoWidget->hide();
	playerStatus = false;
	showFullMenuMode = false;
	downFinish = false;
	positionMove = false;
	fullShow = false;
	downSubtitleMode = false;
	ui.frame_3->hide();
	installEventFilter(this); //安装过滤器  


	//初始化电影搜刮器
	downMovieInfo = new DownMovieInfo(ui.label_4, ui.label_5);
	downMovieInfoThread = new QThread(this);
	downMovieInfo->moveToThread(downMovieInfoThread);
	connect(downMovieInfoThread, &QThread::started, downMovieInfo, &DownMovieInfo::run);
	connect(downMovieInfo, &DownMovieInfo::loadFinish, downMovieInfoThread, &QThread::quit);
	connect(downMovieInfo, &DownMovieInfo::loadStatus, this, &video::showMessageLabel);
	connect(downMovieInfo, &DownMovieInfo::readTextPlot, ui.textBrowser, &QTextBrowser::setText);

	right_2 = new QMenu(this);
	addAction_2 = new QAction(QIcon(":/images/openFile.png"), QStringLiteral("添加目录"));
	deleteAction_2 = new QAction(QIcon(":/images/delete.png"), QStringLiteral("删除目录"));
	ui.listWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);  //设置菜单策略
	connect(ui.listWidget_2, &QListView::customContextMenuRequested, this, &video::showRightMenu_2);
	connect(right_2, &QMenu::triggered, this, &video::listWidgetOption_2);
	//列表项选中状态改变信号
	connect(ui.listWidget, &QListWidget::currentRowChanged, this, &video::showMovieInfo);
	connect(ui.listWidget_2, &QListWidget::currentRowChanged, this, &video::showPathFile);
	//双击项目
	connect(ui.listWidget, &QListWidget::doubleClicked, this, &video::playerMovie);

	//按键操作信号
	connect(videoWidget, &VideoWidget::keyPress, this, &video::keyPress);

	//关于介绍
	connect(ui.aboutAction, &QAction::triggered, [&]() {QMessageBox::about(this, QStringLiteral("关于"), QStringLiteral("开发作者: 神秘\n联系QQ: 735609378\n当前版本: v1.2\n")); });

	//初始化加载字幕线程
	subtitle = new subtitleThread(this);
	label = new QLabel;
	label->setAttribute(Qt::WA_TranslucentBackground);  //设置背景透明
	label->setStyleSheet("color:white;");  //文字颜色白色
	label->setAlignment(Qt::AlignHCenter);  //文字居中
	//设置字号
	QFont ft;
	ft.setPointSize(16);
	label->setFont(ft);
	connect(subtitle, &subtitleThread::readSubtitleText, this, &video::readSubtitleText);
	connect(subtitle, &subtitleThread::loadStatus, this, &video::showMessageLabel);
	connect(subtitle, &subtitleThread::haveFound, this, &video::loadLocalSubtitleFile); //如果找到本地字幕文件后加载
	connect(subtitle, &subtitleThread::finished, this, &video::downFinished);  //字幕下载完成 字幕加载线程成功退出后重新加载字幕

	//初始化字幕下载线程
	download = new DownloadThread;
	downThread = new QThread(this);
	download->moveToThread(downThread);
	connect(downThread, &QThread::started, download, &DownloadThread::run);
	connect(download, &DownloadThread::loadStatus, downThread, &QThread::quit);
	connect(ui.autoDownAction, &QAction::triggered, this, &video::autoLoadSubtitle);
	connect(ui.searchAction, &QAction::triggered, this, &video::searchLoadSubtitle);
	connect(download, &DownloadThread::readFileName, this, &video::loadSubtitleFile);
	connect(download, &DownloadThread::loadStatus, this, &video::showMessageLabel);
	connect(downThread, &QThread::finished, this, &video::startSearch);
	//注册自定义类
	qRegisterMetaType<QList<QStringList *>>("QList<QStringList *>");
	connect(download, &DownloadThread::readListWidget, this, &video::showQListWidget);
	connect(listWidget, &ListWidget::closeDown, this, &video::closeDownWighet);

	//初始化字幕选择下载窗口
	listWidget = new ListWidget;
	connect(listWidget, &ListWidget::currentNum, download, &DownloadThread::loadRowDown);

	//显示字幕
	connect(ui.showAction, &QAction::triggered, this, &video::showSubtitle);


	//播放进度
	connect(player, &QMediaPlayer::positionChanged, this, &video::setSliderValue);
	connect(ui.horizontalSlider, &QSlider::sliderMoved, player,&QMediaPlayer::setPosition);
	connect(ui.horizontalSlider, &QSlider::sliderPressed, this, &video::openPositionMove);  //滑块按下信号
	connect(ui.horizontalSlider, &QSlider::sliderReleased, this, &video::closePositionMove);  //滑块释放信号
	connect(player, &QMediaPlayer::positionChanged, subtitle, &subtitleThread::readTime);
	//播放错误
	connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [this](QMediaPlayer::Error error) { 
		ui.statusBar->showMessage(player->errorString(), 1000);
	});

	//视频音量
	connect(ui.horizontalSlider_2, &QSlider::valueChanged, player, &QMediaPlayer::setVolume);
	connect(ui.horizontalSlider_2, &QSlider::valueChanged, this, &video::setVolumeIcon);

	//文件打开
	connect(ui.openAction, &QAction::triggered, this, &video::openFile);


	//全屏打开关闭信号
	connect(videoWidget, &VideoWidget::showFull, this, &video::showFull);
	connect(videoWidget, &VideoWidget::hideFull, this, &video::hideFull);
	connect(videoWidget, &VideoWidget::showMenu, this, &video::showFullMenu);
	connect(videoWidget, &VideoWidget::hideMeun, this, &video::hideFullMenu);

	//初始化全屏菜单显示时间定时器
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &video::hideFullMenu);

	//打开字幕文件
	connect(ui.openAction_2, &QAction::triggered, this, &video::openSubtitleFile);

	//加载视频信号
	connect(player, &QMediaPlayer::mediaStatusChanged, this, &video::loadMedia);

	//字幕时间
	connect(ui.addAction, &QAction::triggered, this, &video::addSubtitleTime);
	connect(ui.lowerAction, &QAction::triggered, this, &video::lowerSubtitleTime);

	//最近文件菜单
	connect(ui.menu_5, &QMenu::triggered, this, &video::openRecentFile);

	//资源目录显示
	connect(ui.listAction, &QAction::toggled, this, &video::showList);

	readSettings();  //读取配置文件
}


//播放按钮
void video::on_pushButton_clicked()
{
	if (videoWidget->isHidden()) {
		videoWidget->show();
	}

	if (!playerStatus) {
		player->play();
		playerStatus = true;
		ui.horizontalSlider->setMaximum(player->duration());
		player->setVolume(ui.horizontalSlider_2->value());
		ui.label->setText(QTime::fromMSecsSinceStartOfDay(player->duration()).toString("hh:mm:ss"));
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/pause.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui.pushButton->setIcon(icon);
	}
	else {
		player->pause();
		playerStatus = false;
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui.pushButton->setIcon(icon);
	}
}

//析构
video::~video()
{
	delete listWidget;
	writeSettings();  //写入配置文件
}

//读取配置文件
void video::readSettings()
{
	QSettings settings("config.ini", QSettings::IniFormat);

	settings.beginGroup("option");
	ui.autoDownAction->setChecked(settings.value("autoDown").toBool());
	if (!settings.value("volume").isNull())
		ui.horizontalSlider_2->setValue(settings.value("volume").toInt());
	ui.listAction->setChecked(settings.value("folderList").toBool());
	settings.endGroup();

	settings.beginGroup("RecentFile");
	fileNameList = settings.value("file").toStringList();
	if (fileNameList.size() > 0)
		ui.menu_5->clear();
	for (int i = 0; i < fileNameList.size(); i++) {
		ui.menu_5->addAction(QIcon(":/images/video.png"), fileNameList.at(i));
	}
	folderList = settings.value("folder").toStringList();
	for (int i = 0; i < folderList.size(); i++) {
		QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/fileList.png"), folderList.at(i).split("/").at(folderList.at(i).split("/").size() - 1));
		ui.listWidget_2->addItem(item);
	}
	settings.endGroup();
}

//写入配置文件
void video::writeSettings()
{
	QSettings settings("config.ini", QSettings::IniFormat);

	settings.beginGroup("option");
	settings.setValue("autoDown", ui.autoDownAction->isChecked());
	settings.setValue("volume", ui.horizontalSlider_2->value());
	settings.setValue("folderList", ui.listAction->isChecked());
	settings.endGroup();

	settings.beginGroup("RecentFile");
	settings.setValue("file", fileNameList);
	settings.setValue("folder", folderList);
	settings.endGroup();
}

//静音
void video::on_pushButton_2_clicked()
{
	
	if (ui.horizontalSlider_2->value() > 0) {
		volume = ui.horizontalSlider_2->value();
		ui.horizontalSlider_2->setValue(0);
	}
	else {
		ui.horizontalSlider_2->setValue(volume);
	}
}

//设置进度
void video::setSliderValue(qint64 position)
{
	ui.label_2->setText(QTime::fromMSecsSinceStartOfDay(position).toString("hh:mm:ss"));
	if (!positionMove)
		ui.horizontalSlider->setValue(position);
}


//开启进度条移动状态
void video::openPositionMove()
{
	positionMove = true;
}

//关闭进度条移动状态
void video::closePositionMove()
{
	positionMove = false;
}

//打开文件
void video::openFile()
{
	player->pause();
	playerStatus = false;
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui.pushButton->setIcon(icon);
	fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择视频文件"), "./", QStringLiteral("视频文件(*avi *dat *mpg *mp4 *rmvb *rm *mov *mkv *3gp *m2ts *wmv *ts *tp *amv *dmv *flv *swf)"));
	if (fileName.isEmpty()) {
		videoWidget->hide();
		ui.statusBar->showMessage(QStringLiteral("未选择文件"), 1000);
		ui.pushButton->setEnabled(false);
	}
	else {
		if (fileNameList.size() < 10) {
			fileNameList << fileName;
		}
		else {
			fileNameList.removeAt(0);
			ui.menu_5->removeAction(ui.menu_5->actions().at(0));
		}

		if (fileNameList.size() == 1) {
			ui.menu_5->clear();
			ui.menu_5->addAction(QIcon(":/images/video.png"), fileName);
		}
		else {
			ui.menu_5->addAction(QIcon(":/images/video.png"), fileName);
		}
		
		subtitle->exit();
		subtitle->clearSubtitle();
		downThread->exit();
		label->clear();
		this->setWindowTitle(QStringLiteral("神秘快播  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
		player->setMedia(QUrl::fromLocalFile(fileName));
		ui.statusBar->showMessage(QStringLiteral("打开文件成功"), 1000);
		ui.pushButton->setEnabled(true);
	}
}


//显示播放菜单
void video::showFullMenu()
{
	if (!showFullMenuMode) {
		ui.widget_2->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint); //置顶 不显示标题栏
		ui.widget_2->move(videoWidget->frameSize().rwidth() / 2 - ui.widget_2->frameSize().rwidth() / 2, videoWidget->frameSize().rheight() * 0.95);
		ui.widget_2->show();
		showFullMenuMode = true;
		timer->start(10000);
	}
}

//隐藏全屏菜单
void video::hideFullMenu()
{
	if (showFullMenuMode) {
		timer->stop();
		ui.widget_2->setParent(this);
		ui.gridLayout_2->addWidget(ui.widget_2, 1, 0, 1, 1);
		showFullMenuMode = false;
	}
}

//隐藏全屏
void video::hideFull()
{
	fullShow = false;
	ui.gridLayout->addWidget(ui.widget, 0, 0, 1, 1);
	ui.gridLayout_3->addWidget(ui.widget, 0, 0, 1, 1);
	this->show();
	if (ui.showAction->isChecked()) {
		//设置字号
		QFont ft;
		ft.setPointSize(16);
		label->setFont(ft);
		label->setParent(this);
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
		label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //置顶 不显示标题栏 不显示任务栏
		label->show();
	}
}

//显示全屏
void video::showFull()
{
	fullShow = true;
	this->hide();
	if (ui.showAction->isChecked()) {
		//设置字号
		QFont ft;
		ft.setPointSize(38);
		label->setFont(ft);
		label->setParent(ui.widget);
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(0, videoWidget->frameSize().rheight() * 0.9);
		label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //置顶 不显示标题栏 不显示任务栏
		label->show();
	}
}

//打开字幕文件
void video::openSubtitleFile()
{
	fileSubtitle = QFileDialog::getOpenFileName(this, QStringLiteral("选择字幕文件"), "./", QStringLiteral("字幕文件(*srt *ass)"));
	if (fileSubtitle.isEmpty()) {
		ui.statusBar->showMessage(QStringLiteral("未选择文件"), 1000);
	}
	else {
		subtitle->exit();
		ui.statusBar->showMessage(QStringLiteral("打开文件成功"), 1000);
		subtitle->setFileName(fileSubtitle);
		ui.showAction->setChecked(true);
		showSubtitle(true);
	}
}

//显示字幕
void video::showSubtitle(bool checked)
{
	if (playerStatus) {
		if (checked) {
			subtitle->start();
			label->setParent(this);
			label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
			label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
			label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //置顶 不显示标题栏 不显示任务栏
			label->show();
			//设置窗口置顶
			//label->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool); //不显示标题栏 不显示任务栏
			//SetWindowPos(HWND(label->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);  //置顶
			//label->show();
		}
		else {
			label->clear();
			label->hide();
			subtitle->clearSubtitle();
			subtitle->exit();
		}
	}		
}

//读取字幕
void video::readSubtitleText(const QString & str)
{
	label->setText(str);
}

//设置音量图标
void video::setVolumeIcon(int position)
{
	if (position == 0) {
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/volumeClose.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui.pushButton_2->setIcon(icon);
	}
	else {
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/volumeOpen.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui.pushButton_2->setIcon(icon);
	}
}

//按键操作
void video::keyPress(int key)
{
	if (key == Qt::Key_Up) {
		ui.horizontalSlider_2->setValue(ui.horizontalSlider_2->value() + 10 < 100 ? ui.horizontalSlider_2->value() + 10 : 100);
	}
	else if (key == Qt::Key_Down) {
		ui.horizontalSlider_2->setValue(ui.horizontalSlider_2->value() - 10 > 0 ? ui.horizontalSlider_2->value() - 10 : 0);
	}
	else if (key == Qt::Key_Left) {
		player->setPosition(player->position() - qint64(30000) > qint64(0) ? player->position() - qint64(30000) : qint64(0));
	}
	else if (key == Qt::Key_Right) {
		player->setPosition(player->position() + qint64(30000) < player->duration() ? player->position() + qint64(30000) : player->duration());
	}
}

//加载视频完成
void video::loadMedia(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadedMedia) {
		//audioList = player->supportedCustomAudioRoles();
		ui.statusBar->showMessage(QStringLiteral("加载视频完成"), 1000);
		on_pushButton_clicked();
		download->setFileName(fileName);
		subtitle->findFile(fileName);
	}
	else if (status == QMediaPlayer::InvalidMedia) {
		playerStatus = false;
	}
}

//网络下载完成加载字幕文件
void video::loadSubtitleFile(const QString & str)
{
	fileSubtitle = str;
	if (fileSubtitle.isEmpty()) {
		ui.statusBar->showMessage(QStringLiteral("在线下载字幕失败"), 1000);
	}
	else {
		if (subtitle->isRunning()) {
			subtitle->exit();
			subtitle->setFileName(fileSubtitle);
			downFinish = true;
		}
		else {
			subtitle->setFileName(fileSubtitle);
			ui.showAction->setChecked(true);
			showSubtitle(true);
		}
	}
}

//找到本地字幕文件后加载本地字幕文件
void video::loadLocalSubtitleFile(bool status)
{
	if (status) {
		subtitle->exit();
		ui.showAction->setChecked(true);
		showSubtitle(true);
	}
	else {
		if (ui.autoDownAction->isChecked()) {
			download->setDownMode(0);
			downThread->start();
		}
	}
}

//自动在线匹配字幕开始
void video::autoLoadSubtitle(bool checked)
{
	if (playerStatus) {
		if (checked) {
			download->setDownMode(0);
			downThread->start();
		}
		else
			downThread->exit();
	}
}

//搜索匹配在线字幕
void video::searchLoadSubtitle(bool checked)
{
	if (playerStatus) {
		if (downThread->isRunning()) {
			downSubtitleMode = true;
			downThread->exit();
		}
		else {
			download->setDownMode(1);
			downThread->start();
		}
	}
	else {
		ui.statusBar->showMessage(QStringLiteral("当前没有播放视频"), 1000);
	}
}

//开始搜索
void video::startSearch()
{
	if (downSubtitleMode) {
		download->setDownMode(1);
		downThread->start();
		downSubtitleMode = false;
	}
}

//显示消息
void video::showMessageLabel(const QString & str)
{
	ui.statusBar->showMessage(str, 1000);
}

//显示列表窗口
void video::showQListWidget(QList<QStringList*> file)
{
	listWidget->showQListWidget(file);
	listWidget->show();
	fullShow = true;
}

//关闭下载字幕窗口
void video::closeDownWighet()
{
	fullShow = false;
}

//字幕时间提前
void video::addSubtitleTime()
{
	int time = subtitle->addTime();
	ui.timeAction->setText(QStringLiteral("当前%1ms").arg(time));
	ui.statusBar->showMessage(QStringLiteral("字幕提前100ms, 当前时间%1ms").arg(time), 1000);
}

//字幕时间延迟
void video::lowerSubtitleTime()
{
	int time = subtitle->lowerTime();
	ui.timeAction->setText(QStringLiteral("当前%1ms").arg(time));
	ui.statusBar->showMessage(QStringLiteral("字幕延迟100ms, 当前时间%1ms").arg(time), 1000);
}

//字幕下载完成后 字幕线程成功退出
void video::downFinished()
{
	if (downFinish) {
		ui.showAction->setChecked(true);
		showSubtitle(true);
		downFinish=false;
	}
}

//打开最近文件
void video::openRecentFile(QAction * action)
{
	player->pause();
	playerStatus = false;
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui.pushButton->setIcon(icon);
	fileName = action->text();
	subtitle->exit();
	subtitle->clearSubtitle();
	downThread->exit();
	label->clear();
	this->setWindowTitle(QStringLiteral("神秘快播  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
	player->setMedia(QUrl::fromLocalFile(fileName));
	ui.statusBar->showMessage(QStringLiteral("打开文件成功"), 1000);
	ui.pushButton->setEnabled(true);
}

//是否显示目录
void video::showList(bool checked)
{
	if (checked) {
		ui.frame_3->show();
	}
	else {
		ui.frame_3->hide();
	}
}

//显示右键菜单
void video::showRightMenu_2(const QPoint & pos)
{
	QModelIndex index = ui.listWidget_2->indexAt(pos);
	if (index.isValid()) {
		right_2->clear();
		right_2->addAction(addAction_2);
		right_2->addAction(deleteAction_2);
		right_2->exec(QCursor::pos());
	}
	else {
		right_2->clear();
		right_2->addAction(addAction_2);
		right_2->exec(QCursor::pos());
	}
}

//菜单关联
void video::listWidgetOption_2(QAction * action)
{
	if (action->text() == QStringLiteral("添加目录")) {
		QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("选择文件夹"), "./");
		if (path.isEmpty()) {
			ui.statusBar->showMessage(QStringLiteral("未选择目录"), 1000);
		}
		else {
			folderList << path;
			QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/fileList.png"), path.split("/").at(path.split("/").size() - 1));
			ui.listWidget_2->addItem(item);
		}
	}
	else if (action->text() == QStringLiteral("删除目录")) {
		int row = ui.listWidget_2->currentIndex().row();  //获取选中行
		QListWidgetItem *itme = ui.listWidget_2->takeItem(row);
		delete itme;
		folderList.removeAt(row);
	}
}

//显示目录文件
void video::showPathFile(int row)
{
	if (row > -1) {
		QDir dir(folderList.at(row));
		QStringList list;
		list << "*.avi" << "*.dat" << "*.mpg" << "*.mp4" << "*.rmvb" << "*.rm" << "*.mov" << "*.mkv" << "*.3gp" << "*.m2ts" << "*.wmv" << "*.ts" << "*.tp" << "*.amv" << "*.dmv" << "*.flv" << "*.swf";
		dir.setNameFilters(list);
		list = dir.entryList();
		ui.listWidget->clear();
		folderFileList.clear();
		folderFilePath.clear();
		for (int i = 0; i < list.size(); i++) {
			QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/video.png"), list.at(i));
			ui.listWidget->addItem(item);
			folderFileList << folderList.at(row) + "/" + list.at(i);
			folderFilePath[list.at(i)] = folderFileList.at(i);
		}
	}
}

//显示电影信息
void video::showMovieInfo(int row)
{
	if (row > -1) {
		if (!downMovieInfoThread->isRunning()) {
			downMovieInfo->setFileName(folderFileList.at(row).split("/").at(folderFileList.at(row).split("/").size() - 1));
			downMovieInfoThread->start();
		}
		else {
			ui.statusBar->showMessage(QStringLiteral("切换太频繁, 上一个查询还在进行中"), 1000);
		}
	}
}

//双击目录电影项播放视频
void video::playerMovie(const QModelIndex & index)
{
	if (!folderFilePath.value(index.data().toString()).isNull()) {
		player->pause();
		playerStatus = false;
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui.pushButton->setIcon(icon);
		fileName = folderFilePath.value(index.data().toString());
		subtitle->exit();
		subtitle->clearSubtitle();
		downThread->exit();
		label->clear();
		this->setWindowTitle(QStringLiteral("神秘快播  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
		player->setMedia(QUrl::fromLocalFile(fileName));
		ui.statusBar->showMessage(QStringLiteral("打开文件成功"), 1000);
		ui.pushButton->setEnabled(true);
	}
	else {
		ui.statusBar->showMessage(QStringLiteral("打开视频文件失败"), 1000);
	}
}


//窗口移动事件
void video::moveEvent(QMoveEvent * event)
{
	if (ui.showAction->isChecked()) {
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
	}
}

//窗口大小改变事件
void video::resizeEvent(QResizeEvent * event)
{
	if (ui.showAction->isChecked()) {
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
	}	
}

//窗口关闭事件
void video::closeEvent(QCloseEvent * event)
{
	label->hide();
	subtitle->exit();
	downThread->exit();
	downMovieInfoThread->exit();
	listWidget->close();
}

//事件过滤器 
bool video::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this) {  //如果是本窗口
		if (!fullShow) {
			if (event->type() == QEvent::WindowActivate) {  //如果窗口被激活
				if (ui.showAction->isChecked()) {
					label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //置顶 不显示标题栏 不显示任务栏
				}
				return true;
			}
			else if (event->type() == QEvent::WindowDeactivate) {  //如果窗口被停用
				if (ui.showAction->isChecked()) {
					SetWindowPos(HWND(label->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW); //取消置顶	
				}
				return false;
			}
		}
	}
	return false;
}
