#include "ListWidget.h"

ListWidget::ListWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//列表项选中状态改变信号
	connect(ui.listWidget, &QListWidget::currentRowChanged, this, &ListWidget::setPushButtonStatus);
}

ListWidget::~ListWidget()
{
}

//显示列表窗口
void ListWidget::showQListWidget(QList<QStringList*> file)
{
	ui.listWidget->clear();
	for (int i = 0; i < file.size(); i++) {
		ui.listWidget->addItem(file[i]->at(1));
	}
}

//设置按钮状态
void ListWidget::setPushButtonStatus(int row)
{
	if (row > -1) {
		ui.pushButton->setEnabled(true);
	}
}

//窗口关闭事件
void ListWidget::closeEvent(QCloseEvent * event)
{
	emit closeDown();
}

//下载按钮
void ListWidget::on_pushButton_clicked()
{
	if (ui.listWidget->currentRow() > -1) {
		emit currentNum(ui.listWidget->currentRow());
		this->hide();
	}
}