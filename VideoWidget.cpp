#include "VideoWidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent)
	: QVideoWidget(parent)
{
}

VideoWidget::~VideoWidget()
{
}

//����ƶ��¼�
void VideoWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (isFullScreen()) {
		if (event->pos().y() + 200 > this->frameSize().rheight()) {
			emit showMenu();
		}
	}
}

//���˫���¼�
void VideoWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (isFullScreen()) {
		setFullScreen(false);
		emit hideMeun();
		emit hideFull();
	}
	else {
		setFullScreen(true);
		emit showFull();
	}
}

//���������¼�
void VideoWidget::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Up) {
		emit keyPress(event->key());
	}
	else if (event->key() == Qt::Key_Down) {
		emit keyPress(event->key());
	}
	else if (event->key() == Qt::Key_Left) {
		emit keyPress(event->key());
	}
	else if (event->key() == Qt::Key_Right) {
		emit keyPress(event->key());
	}
}

