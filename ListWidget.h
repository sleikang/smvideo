#pragma once

#include <QWidget>
#include "ui_ListWidget.h"

class ListWidget : public QWidget
{
	Q_OBJECT

public:
	ListWidget(QWidget *parent = Q_NULLPTR);
	~ListWidget();
	void showQListWidget(QList<QStringList *> file);  //显示列表窗口

private:
	Ui::ListWidget ui;

private slots:
	void on_pushButton_clicked();   //下载按钮
	void setPushButtonStatus(int row);  //设置按钮状态

signals:
	void currentNum(int row);  //当前选择信号
	void closeDown();  //关闭窗口下载信号

protected:
	void closeEvent(QCloseEvent *event); //窗口关闭事件
};
