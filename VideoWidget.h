#pragma once

#include <QVideoWidget>

class VideoWidget : public QVideoWidget
{
	Q_OBJECT

public:
	VideoWidget(QWidget *parent = nullptr);
	~VideoWidget();

protected:
	void mouseMoveEvent(QMouseEvent *event);  //����ƶ��¼�
	void mouseDoubleClickEvent(QMouseEvent *event);  //���˫���¼�
	void keyPressEvent(QKeyEvent *event);  //���������¼�

signals:
	void showMenu();
	void hideMeun();
	void hideFull();
	void showFull();
	void keyPress(int key);  //����
};
