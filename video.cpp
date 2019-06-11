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
	videoWidget->setFocusPolicy(Qt::ClickFocus);  //���ð�������
	player->setVideoOutput(videoWidget);
	gridLayout = new QGridLayout(ui.widget);  //��Ƶ���ŵ����ڲ���
	gridLayout->addWidget(videoWidget);
	gridLayout->setContentsMargins(0, 0, 0, 0); //���ò��ֱ߾�
	ui.gridLayout->setContentsMargins(0, 0, 0, 0);
	videoWidget->hide();
	playerStatus = false;
	showFullMenuMode = false;
	downFinish = false;
	positionMove = false;
	fullShow = false;
	downSubtitleMode = false;
	ui.frame_3->hide();
	installEventFilter(this); //��װ������  


	//��ʼ����Ӱ�ѹ���
	downMovieInfo = new DownMovieInfo(ui.label_4, ui.label_5);
	downMovieInfoThread = new QThread(this);
	downMovieInfo->moveToThread(downMovieInfoThread);
	connect(downMovieInfoThread, &QThread::started, downMovieInfo, &DownMovieInfo::run);
	connect(downMovieInfo, &DownMovieInfo::loadFinish, downMovieInfoThread, &QThread::quit);
	connect(downMovieInfo, &DownMovieInfo::loadStatus, this, &video::showMessageLabel);
	connect(downMovieInfo, &DownMovieInfo::readTextPlot, ui.textBrowser, &QTextBrowser::setText);

	right_2 = new QMenu(this);
	addAction_2 = new QAction(QIcon(":/images/openFile.png"), QStringLiteral("���Ŀ¼"));
	deleteAction_2 = new QAction(QIcon(":/images/delete.png"), QStringLiteral("ɾ��Ŀ¼"));
	ui.listWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);  //���ò˵�����
	connect(ui.listWidget_2, &QListView::customContextMenuRequested, this, &video::showRightMenu_2);
	connect(right_2, &QMenu::triggered, this, &video::listWidgetOption_2);
	//�б���ѡ��״̬�ı��ź�
	connect(ui.listWidget, &QListWidget::currentRowChanged, this, &video::showMovieInfo);
	connect(ui.listWidget_2, &QListWidget::currentRowChanged, this, &video::showPathFile);
	//˫����Ŀ
	connect(ui.listWidget, &QListWidget::doubleClicked, this, &video::playerMovie);

	//���������ź�
	connect(videoWidget, &VideoWidget::keyPress, this, &video::keyPress);

	//���ڽ���
	connect(ui.aboutAction, &QAction::triggered, [&]() {QMessageBox::about(this, QStringLiteral("����"), QStringLiteral("��������: ����\n��ϵQQ: 735609378\n��ǰ�汾: v1.2\n")); });

	//��ʼ��������Ļ�߳�
	subtitle = new subtitleThread(this);
	label = new QLabel;
	label->setAttribute(Qt::WA_TranslucentBackground);  //���ñ���͸��
	label->setStyleSheet("color:white;");  //������ɫ��ɫ
	label->setAlignment(Qt::AlignHCenter);  //���־���
	//�����ֺ�
	QFont ft;
	ft.setPointSize(16);
	label->setFont(ft);
	connect(subtitle, &subtitleThread::readSubtitleText, this, &video::readSubtitleText);
	connect(subtitle, &subtitleThread::loadStatus, this, &video::showMessageLabel);
	connect(subtitle, &subtitleThread::haveFound, this, &video::loadLocalSubtitleFile); //����ҵ�������Ļ�ļ������
	connect(subtitle, &subtitleThread::finished, this, &video::downFinished);  //��Ļ������� ��Ļ�����̳߳ɹ��˳������¼�����Ļ

	//��ʼ����Ļ�����߳�
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
	//ע���Զ�����
	qRegisterMetaType<QList<QStringList *>>("QList<QStringList *>");
	connect(download, &DownloadThread::readListWidget, this, &video::showQListWidget);
	connect(listWidget, &ListWidget::closeDown, this, &video::closeDownWighet);

	//��ʼ����Ļѡ�����ش���
	listWidget = new ListWidget;
	connect(listWidget, &ListWidget::currentNum, download, &DownloadThread::loadRowDown);

	//��ʾ��Ļ
	connect(ui.showAction, &QAction::triggered, this, &video::showSubtitle);


	//���Ž���
	connect(player, &QMediaPlayer::positionChanged, this, &video::setSliderValue);
	connect(ui.horizontalSlider, &QSlider::sliderMoved, player,&QMediaPlayer::setPosition);
	connect(ui.horizontalSlider, &QSlider::sliderPressed, this, &video::openPositionMove);  //���鰴���ź�
	connect(ui.horizontalSlider, &QSlider::sliderReleased, this, &video::closePositionMove);  //�����ͷ��ź�
	connect(player, &QMediaPlayer::positionChanged, subtitle, &subtitleThread::readTime);
	//���Ŵ���
	connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [this](QMediaPlayer::Error error) { 
		ui.statusBar->showMessage(player->errorString(), 1000);
	});

	//��Ƶ����
	connect(ui.horizontalSlider_2, &QSlider::valueChanged, player, &QMediaPlayer::setVolume);
	connect(ui.horizontalSlider_2, &QSlider::valueChanged, this, &video::setVolumeIcon);

	//�ļ���
	connect(ui.openAction, &QAction::triggered, this, &video::openFile);


	//ȫ���򿪹ر��ź�
	connect(videoWidget, &VideoWidget::showFull, this, &video::showFull);
	connect(videoWidget, &VideoWidget::hideFull, this, &video::hideFull);
	connect(videoWidget, &VideoWidget::showMenu, this, &video::showFullMenu);
	connect(videoWidget, &VideoWidget::hideMeun, this, &video::hideFullMenu);

	//��ʼ��ȫ���˵���ʾʱ�䶨ʱ��
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &video::hideFullMenu);

	//����Ļ�ļ�
	connect(ui.openAction_2, &QAction::triggered, this, &video::openSubtitleFile);

	//������Ƶ�ź�
	connect(player, &QMediaPlayer::mediaStatusChanged, this, &video::loadMedia);

	//��Ļʱ��
	connect(ui.addAction, &QAction::triggered, this, &video::addSubtitleTime);
	connect(ui.lowerAction, &QAction::triggered, this, &video::lowerSubtitleTime);

	//����ļ��˵�
	connect(ui.menu_5, &QMenu::triggered, this, &video::openRecentFile);

	//��ԴĿ¼��ʾ
	connect(ui.listAction, &QAction::toggled, this, &video::showList);

	readSettings();  //��ȡ�����ļ�
}


//���Ű�ť
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

//����
video::~video()
{
	delete listWidget;
	writeSettings();  //д�������ļ�
}

//��ȡ�����ļ�
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

//д�������ļ�
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

//����
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

//���ý���
void video::setSliderValue(qint64 position)
{
	ui.label_2->setText(QTime::fromMSecsSinceStartOfDay(position).toString("hh:mm:ss"));
	if (!positionMove)
		ui.horizontalSlider->setValue(position);
}


//�����������ƶ�״̬
void video::openPositionMove()
{
	positionMove = true;
}

//�رս������ƶ�״̬
void video::closePositionMove()
{
	positionMove = false;
}

//���ļ�
void video::openFile()
{
	player->pause();
	playerStatus = false;
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui.pushButton->setIcon(icon);
	fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ����Ƶ�ļ�"), "./", QStringLiteral("��Ƶ�ļ�(*avi *dat *mpg *mp4 *rmvb *rm *mov *mkv *3gp *m2ts *wmv *ts *tp *amv *dmv *flv *swf)"));
	if (fileName.isEmpty()) {
		videoWidget->hide();
		ui.statusBar->showMessage(QStringLiteral("δѡ���ļ�"), 1000);
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
		this->setWindowTitle(QStringLiteral("���ؿ첥  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
		player->setMedia(QUrl::fromLocalFile(fileName));
		ui.statusBar->showMessage(QStringLiteral("���ļ��ɹ�"), 1000);
		ui.pushButton->setEnabled(true);
	}
}


//��ʾ���Ų˵�
void video::showFullMenu()
{
	if (!showFullMenuMode) {
		ui.widget_2->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint); //�ö� ����ʾ������
		ui.widget_2->move(videoWidget->frameSize().rwidth() / 2 - ui.widget_2->frameSize().rwidth() / 2, videoWidget->frameSize().rheight() * 0.95);
		ui.widget_2->show();
		showFullMenuMode = true;
		timer->start(10000);
	}
}

//����ȫ���˵�
void video::hideFullMenu()
{
	if (showFullMenuMode) {
		timer->stop();
		ui.widget_2->setParent(this);
		ui.gridLayout_2->addWidget(ui.widget_2, 1, 0, 1, 1);
		showFullMenuMode = false;
	}
}

//����ȫ��
void video::hideFull()
{
	fullShow = false;
	ui.gridLayout->addWidget(ui.widget, 0, 0, 1, 1);
	ui.gridLayout_3->addWidget(ui.widget, 0, 0, 1, 1);
	this->show();
	if (ui.showAction->isChecked()) {
		//�����ֺ�
		QFont ft;
		ft.setPointSize(16);
		label->setFont(ft);
		label->setParent(this);
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
		label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //�ö� ����ʾ������ ����ʾ������
		label->show();
	}
}

//��ʾȫ��
void video::showFull()
{
	fullShow = true;
	this->hide();
	if (ui.showAction->isChecked()) {
		//�����ֺ�
		QFont ft;
		ft.setPointSize(38);
		label->setFont(ft);
		label->setParent(ui.widget);
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(0, videoWidget->frameSize().rheight() * 0.9);
		label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //�ö� ����ʾ������ ����ʾ������
		label->show();
	}
}

//����Ļ�ļ�
void video::openSubtitleFile()
{
	fileSubtitle = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ����Ļ�ļ�"), "./", QStringLiteral("��Ļ�ļ�(*srt *ass)"));
	if (fileSubtitle.isEmpty()) {
		ui.statusBar->showMessage(QStringLiteral("δѡ���ļ�"), 1000);
	}
	else {
		subtitle->exit();
		ui.statusBar->showMessage(QStringLiteral("���ļ��ɹ�"), 1000);
		subtitle->setFileName(fileSubtitle);
		ui.showAction->setChecked(true);
		showSubtitle(true);
	}
}

//��ʾ��Ļ
void video::showSubtitle(bool checked)
{
	if (playerStatus) {
		if (checked) {
			subtitle->start();
			label->setParent(this);
			label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
			label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
			label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //�ö� ����ʾ������ ����ʾ������
			label->show();
			//���ô����ö�
			//label->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool); //����ʾ������ ����ʾ������
			//SetWindowPos(HWND(label->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);  //�ö�
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

//��ȡ��Ļ
void video::readSubtitleText(const QString & str)
{
	label->setText(str);
}

//��������ͼ��
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

//��������
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

//������Ƶ���
void video::loadMedia(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadedMedia) {
		//audioList = player->supportedCustomAudioRoles();
		ui.statusBar->showMessage(QStringLiteral("������Ƶ���"), 1000);
		on_pushButton_clicked();
		download->setFileName(fileName);
		subtitle->findFile(fileName);
	}
	else if (status == QMediaPlayer::InvalidMedia) {
		playerStatus = false;
	}
}

//����������ɼ�����Ļ�ļ�
void video::loadSubtitleFile(const QString & str)
{
	fileSubtitle = str;
	if (fileSubtitle.isEmpty()) {
		ui.statusBar->showMessage(QStringLiteral("����������Ļʧ��"), 1000);
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

//�ҵ�������Ļ�ļ�����ر�����Ļ�ļ�
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

//�Զ�����ƥ����Ļ��ʼ
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

//����ƥ��������Ļ
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
		ui.statusBar->showMessage(QStringLiteral("��ǰû�в�����Ƶ"), 1000);
	}
}

//��ʼ����
void video::startSearch()
{
	if (downSubtitleMode) {
		download->setDownMode(1);
		downThread->start();
		downSubtitleMode = false;
	}
}

//��ʾ��Ϣ
void video::showMessageLabel(const QString & str)
{
	ui.statusBar->showMessage(str, 1000);
}

//��ʾ�б���
void video::showQListWidget(QList<QStringList*> file)
{
	listWidget->showQListWidget(file);
	listWidget->show();
	fullShow = true;
}

//�ر�������Ļ����
void video::closeDownWighet()
{
	fullShow = false;
}

//��Ļʱ����ǰ
void video::addSubtitleTime()
{
	int time = subtitle->addTime();
	ui.timeAction->setText(QStringLiteral("��ǰ%1ms").arg(time));
	ui.statusBar->showMessage(QStringLiteral("��Ļ��ǰ100ms, ��ǰʱ��%1ms").arg(time), 1000);
}

//��Ļʱ���ӳ�
void video::lowerSubtitleTime()
{
	int time = subtitle->lowerTime();
	ui.timeAction->setText(QStringLiteral("��ǰ%1ms").arg(time));
	ui.statusBar->showMessage(QStringLiteral("��Ļ�ӳ�100ms, ��ǰʱ��%1ms").arg(time), 1000);
}

//��Ļ������ɺ� ��Ļ�̳߳ɹ��˳�
void video::downFinished()
{
	if (downFinish) {
		ui.showAction->setChecked(true);
		showSubtitle(true);
		downFinish=false;
	}
}

//������ļ�
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
	this->setWindowTitle(QStringLiteral("���ؿ첥  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
	player->setMedia(QUrl::fromLocalFile(fileName));
	ui.statusBar->showMessage(QStringLiteral("���ļ��ɹ�"), 1000);
	ui.pushButton->setEnabled(true);
}

//�Ƿ���ʾĿ¼
void video::showList(bool checked)
{
	if (checked) {
		ui.frame_3->show();
	}
	else {
		ui.frame_3->hide();
	}
}

//��ʾ�Ҽ��˵�
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

//�˵�����
void video::listWidgetOption_2(QAction * action)
{
	if (action->text() == QStringLiteral("���Ŀ¼")) {
		QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("ѡ���ļ���"), "./");
		if (path.isEmpty()) {
			ui.statusBar->showMessage(QStringLiteral("δѡ��Ŀ¼"), 1000);
		}
		else {
			folderList << path;
			QListWidgetItem *item = new QListWidgetItem(QIcon(":/images/fileList.png"), path.split("/").at(path.split("/").size() - 1));
			ui.listWidget_2->addItem(item);
		}
	}
	else if (action->text() == QStringLiteral("ɾ��Ŀ¼")) {
		int row = ui.listWidget_2->currentIndex().row();  //��ȡѡ����
		QListWidgetItem *itme = ui.listWidget_2->takeItem(row);
		delete itme;
		folderList.removeAt(row);
	}
}

//��ʾĿ¼�ļ�
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

//��ʾ��Ӱ��Ϣ
void video::showMovieInfo(int row)
{
	if (row > -1) {
		if (!downMovieInfoThread->isRunning()) {
			downMovieInfo->setFileName(folderFileList.at(row).split("/").at(folderFileList.at(row).split("/").size() - 1));
			downMovieInfoThread->start();
		}
		else {
			ui.statusBar->showMessage(QStringLiteral("�л�̫Ƶ��, ��һ����ѯ���ڽ�����"), 1000);
		}
	}
}

//˫��Ŀ¼��Ӱ�����Ƶ
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
		this->setWindowTitle(QStringLiteral("���ؿ첥  -  %1").arg(fileName.split("/").at(fileName.split("/").size() - 1)));
		player->setMedia(QUrl::fromLocalFile(fileName));
		ui.statusBar->showMessage(QStringLiteral("���ļ��ɹ�"), 1000);
		ui.pushButton->setEnabled(true);
	}
	else {
		ui.statusBar->showMessage(QStringLiteral("����Ƶ�ļ�ʧ��"), 1000);
	}
}


//�����ƶ��¼�
void video::moveEvent(QMoveEvent * event)
{
	if (ui.showAction->isChecked()) {
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
	}
}

//���ڴ�С�ı��¼�
void video::resizeEvent(QResizeEvent * event)
{
	if (ui.showAction->isChecked()) {
		label->resize(videoWidget->frameSize().rwidth(), videoWidget->frameSize().rheight() * 0.1);
		label->move(this->pos().x(), this->pos().y() + videoWidget->frameSize().rheight());
	}	
}

//���ڹر��¼�
void video::closeEvent(QCloseEvent * event)
{
	label->hide();
	subtitle->exit();
	downThread->exit();
	downMovieInfoThread->exit();
	listWidget->close();
}

//�¼������� 
bool video::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this) {  //����Ǳ�����
		if (!fullShow) {
			if (event->type() == QEvent::WindowActivate) {  //������ڱ�����
				if (ui.showAction->isChecked()) {
					label->setWindowFlags(videoWidget->windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool); //�ö� ����ʾ������ ����ʾ������
				}
				return true;
			}
			else if (event->type() == QEvent::WindowDeactivate) {  //������ڱ�ͣ��
				if (ui.showAction->isChecked()) {
					SetWindowPos(HWND(label->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW); //ȡ���ö�	
				}
				return false;
			}
		}
	}
	return false;
}
