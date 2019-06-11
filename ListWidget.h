#pragma once

#include <QWidget>
#include "ui_ListWidget.h"

class ListWidget : public QWidget
{
	Q_OBJECT

public:
	ListWidget(QWidget *parent = Q_NULLPTR);
	~ListWidget();
	void showQListWidget(QList<QStringList *> file);  //��ʾ�б���

private:
	Ui::ListWidget ui;

private slots:
	void on_pushButton_clicked();   //���ذ�ť
	void setPushButtonStatus(int row);  //���ð�ť״̬

signals:
	void currentNum(int row);  //��ǰѡ���ź�
	void closeDown();  //�رմ��������ź�

protected:
	void closeEvent(QCloseEvent *event); //���ڹر��¼�
};
