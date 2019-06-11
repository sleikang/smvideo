#pragma once

#include <QVideoWidget>

class VideoWidget : public QVideoWidget
{
	Q_OBJECT

public:
	VideoWidget(QWidget *parent = nullptr);
	~VideoWidget();

protected:
	void mouseMoveEvent(QMouseEvent *event);  //鼠标移动事件
	void mouseDoubleClickEvent(QMouseEvent *event);  //鼠标双击事件
	void keyPressEvent(QKeyEvent *event);  //按键按下事件

signals:
	void showMenu();
	void hideMeun();
	void hideFull();
	void showFull();
	void keyPress(int key);  //按键
};
