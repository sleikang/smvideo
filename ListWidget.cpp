#include "ListWidget.h"

ListWidget::ListWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//�б���ѡ��״̬�ı��ź�
	connect(ui.listWidget, &QListWidget::currentRowChanged, this, &ListWidget::setPushButtonStatus);
}

ListWidget::~ListWidget()
{
}

//��ʾ�б���
void ListWidget::showQListWidget(QList<QStringList*> file)
{
	ui.listWidget->clear();
	for (int i = 0; i < file.size(); i++) {
		ui.listWidget->addItem(file[i]->at(1));
	}
}

//���ð�ť״̬
void ListWidget::setPushButtonStatus(int row)
{
	if (row > -1) {
		ui.pushButton->setEnabled(true);
	}
}

//���ڹر��¼�
void ListWidget::closeEvent(QCloseEvent * event)
{
	emit closeDown();
}

//���ذ�ť
void ListWidget::on_pushButton_clicked()
{
	if (ui.listWidget->currentRow() > -1) {
		emit currentNum(ui.listWidget->currentRow());
		this->hide();
	}
}